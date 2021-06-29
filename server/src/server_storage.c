#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

#include "../include/server_storage.h"
#include "../include/pthread_utils.h"
#include "../include/icl_hash.h"
#include "../../core/include/list_utils.h"
#include "../../core/include/utils.h"

// Capacità massima del server storage
static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

// Capacità corrente del server storage
static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

// Contatore utilizzato per la politica di rimpiazzamento, viene rimosso il file con il valore fifo più basso
static int TIME_FIFO = 0;

// Dimensione massima raggiunta dallo storage 
static size_t STORAGE_SIZE_MAX = 0;
// Numero di volte che la politica di rimpiazzamento è stata applicata
static int REPLACEMENT_FREQUENCY = 0;

// Struttura dati dello storage
static icl_hash_t *storage;
// Lock globale sullo storage, viene usata soltanto quando viene inserito nello storage un nuovo file o per liberare tutta la struttura dati
static pthread_mutex_t STORAGE_LOCK = PTHREAD_MUTEX_INITIALIZER;
// Lock per modificare in mutua esclusione CURRENT_FILE_AMOUNT e CURRENT_STORAGE_SIZE
static pthread_mutex_t CAPACITY_LOCK = PTHREAD_MUTEX_INITIALIZER;
// Lock per modificare il contatore REPLACEMENT_FREQUENCY in mutua esclusione
static pthread_mutex_t REPLACEMENT_FREQUENCY_LOCK = PTHREAD_MUTEX_INITIALIZER;

// Macro utilizzate dalla funzione di hashing della mappa.
#define BITS_IN_int     ( sizeof(int) * CHAR_BIT )
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       ( ~((unsigned int)(~0) >> ONE_EIGHTH ))

// Funzione di Hashing delle chiavi utilizzata dalla mappa, è la stessa funzione base della libreria,
// l'unica differenza è che la chiave è un file inveche che un char * e la funzione di hashing viene fatta su file->filePath
unsigned int hash_key(void* key) {
    File *file = (File *) key;

    char *datum = (char *) file->filePath;
    unsigned int hash_value, i;

    if(!datum) return 0;

    for (hash_value = 0; *datum; ++datum) {
        hash_value = (hash_value << ONE_EIGHTH) + *datum;
        if ((i = hash_value & HIGH_BITS) != 0)
            hash_value = (hash_value ^ (i >> THREE_QUARTERS)) & ~HIGH_BITS;
    }
    return (hash_value);
}

// Funzione di confronto utilizzata dalla mappa
int hash_value(void *a, void *b) {

    File *fileA = (File *) a;
    File *fileB = (File *) b;

    int res = strcmp(fileA->filePath, fileB->filePath);
    return res == 0 ? 1 : 0;
}

// Funzione di confronto utilizzata dalla lista
int list_compare(void *a, void *b) {
    return *((int *) a) == *((int *) b);
}

// Inizializza la Mappa
void create_storage(size_t fileCapacity, size_t storageCapacity);
// Inserisce un nuovo file all'interno dello storage
int insert_file_storage(int fileDescriptor, char *filePath);
// Rimuove il file dettato dalla politica di rimpiazzamento
int replace_file_storage(char *filePath);
// Rimuove il file selezionato dalla mappa
int remove_file_storage(File *fileToRemove);
// Restituisco il file che ha come path quello passato per parametro
File *get_file(char *filePath);
// Stampo la mappa in modalità debug
void print_storage_debug();
// Stampo la mappa
void print_storage();
// Stampa le statistiche del server
void print_stats();
// Eseguo la free della struttura dati
void destroy_storage();

// Operazioni che gli utenti possono richiedere di fare nello storage
int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock);
void *read_file(int fileDescriptor, char *filePath, int *bufferSize);
char *read_n_file(int nFiles, int *bufferSize);
int write_file(int fileDescriptor, char *filePath, char *fileContent);
int close_file(int fileDescriptor, char *filePath);
int remove_file(int fileDescriptor, char *filePath);
void disconnect_client(int fileDescriptor);

void create_storage(size_t fileCapacity, size_t storageCapacity) {
    // Imposto il limite dello storage dai file config
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    // Inizializzo la mappa
    storage = icl_hash_create(fileCapacity, hash_key, hash_value);
}

// Inserisce un nuovo file all'interno dello storage
int insert_file_storage(int fileDescriptor, char *filePath) {

    // Acquisisco la lock globale sullo storage
    LOCK(&STORAGE_LOCK);

    // Mi calcolo il peso del file
    size_t pathLength = strlen(filePath) + 1;
    size_t contentLength = strlen("") + 1;
    size_t fileSize = pathLength + contentLength;

    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    LOCK(&CAPACITY_LOCK);
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: È stato raggiunto il massimo numero di File nello Storage.\n");
        // Applico politica di rimpiazzamento
        if (replace_file_storage(filePath) == -1) {
            // In caso di errore rilascio la lock e restituisco un errore
            UNLOCK(&STORAGE_LOCK)
            return -1;
        }
    }
    UNLOCK(&CAPACITY_LOCK);

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    LOCK(&CAPACITY_LOCK);
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: È stata raggiunta la dimensione massima dello Storage.\n");
        // Applico politica di rimpiazzamento
        if (replace_file_storage(filePath) == -1) {
            // In caso di errore rilascio la lock e restituisco un errore
            UNLOCK(&STORAGE_LOCK)
            return -1;
        }
        LOCK(&CAPACITY_LOCK);
    }     
    UNLOCK(&CAPACITY_LOCK);

    // Alloco sullo heap la memoria per creare un nuovo file
    File *newFile;
    if ((newFile = (File *) malloc(sizeof(File))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }
   
    // Alloco la memoria per il path assoluto
	char *path;
    if ((path = (char *) malloc(pathLength)) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }

    strncpy(path, filePath, pathLength);
	newFile->filePath = path;

    // Alloco la memoria per il contenuto
    char *content;
    if ((content = (char *) malloc(contentLength)) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }

	strncpy(content, "", contentLength);
	newFile->fileContent = content;

    // Alloco la memoria per la dimensione del file
    size_t *size;
    if ((size = (size_t *) malloc(sizeof(size_t))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }

	*size = fileSize;
	newFile->fileSize = size;

    // Alloco la memoria per la politica di rimpiazzamento
    int *fifo;
    if ((fifo = (int *) malloc(sizeof(int))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }

    *fifo = TIME_FIFO;
    TIME_FIFO++;
    newFile->fifo = fifo;

    // Alloco la memoria per la lock
    pthread_mutex_t *lock;
    if ((lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }

    pthread_mutex_init(lock, NULL);
	newFile->fileLock = lock;

    // Creo la lista e la inizializzo
    Node *usersList;
    create_list(&usersList, list_compare);

    // Inserisco l'utente che ha richiesto la open del nuovo file nella lista
    int *fd;
    if ((fd = (int *) malloc(sizeof(int))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }

    *fd = fileDescriptor;
    add_tail(&usersList, fd);
    
    // Inserisco nella mappa la nuova entry che ha come chiave, il file e come valore, la lista di utenti che hanno eseguto la open
    icl_hash_insert(storage, newFile, usersList);
    
    // Aggiorno la dimensione corrente del file storage
    LOCK(&CAPACITY_LOCK);
    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += *(newFile->fileSize);
    if (CURRENT_STORAGE_SIZE > STORAGE_SIZE_MAX) STORAGE_SIZE_MAX = CURRENT_STORAGE_SIZE;
    UNLOCK(&CAPACITY_LOCK);
    
    // Rilascio la lock globale
    UNLOCK(&STORAGE_LOCK);

    return 0;
}

// Rimuove il file selezionato dalla politica di rimpiazzamento
int replace_file_storage(char *filePath) {

    // Aumento il contatore globale del numero di volte in cui è stata fatta la politica di rimpiazzamento
    LOCK(&REPLACEMENT_FREQUENCY_LOCK);
    REPLACEMENT_FREQUENCY++;
    UNLOCK(&REPLACEMENT_FREQUENCY_LOCK);

    // Cerco nella mappa il file che ha il valore fifo più basso
    int min = INT_MAX;  
    File *fileToRemove = NULL;
    
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL && *(file->fifo) < min && !strcmp(file->filePath, filePath) == 0) {
                min = *(file->fifo);
                fileToRemove = file;
            }
            curr = curr->next;
        }
    }

    // Se non trovo nessun file, significa che la mappa è vuota
    if (fileToRemove == NULL) {
        fprintf(stderr, "ATTENZIONE: Non è possibile rimuovere nessun altro file dallo storage, quindi la richiesta non viene eseguita!\n");
        return -1;
    }
    
    fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);        

    // Rimuovo il file selezionato dalla politica di rimpiazzamento
    remove_file_storage(fileToRemove);

    return 0;
}

// Rimuove dalla mappa il file passato per parametro
int remove_file_storage(File *fileToRemove) {

    // Se il file passato per parametro è NULL, restituisco errore
    if (fileToRemove == NULL) {
        return -1;
    }

    // Altrimenti acquisco la lock sul file che voglio rimuovere
    LOCK(fileToRemove->fileLock);

    // Aggiorno la dimensione corrente dello storage
    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE -= *(fileToRemove->fileSize);
    CURRENT_FILE_AMOUNT--;
    UNLOCK(&CAPACITY_LOCK);

    // Prendo la lista di utenti che hanno fatto la open sulla mappa. icl_hash_find_pointer è come icl_hash_find con la differenza che restituisce il puntatore del valore 
    Node **usersList = (Node **) icl_hash_find_pointer(storage, fileToRemove);

    // Itero la lista gli elementi, liberando anche la memoria
    for (Node *currentList = *usersList; currentList != NULL; ) {
        Node *temp = currentList;
        currentList = currentList->next;
        
        free(temp->value);
        free(temp);
    }

    // Prendo il puntatore della lock
    pthread_mutex_t *lock = fileToRemove->fileLock;
    // Prendo il puntatore che punta al path assoluto
    char *filePath = fileToRemove->filePath;

    // Libero la memoria
    free(fileToRemove->fileContent);
    free(fileToRemove->fileSize);
    free(fileToRemove->fifo);

    // Rimuovo la entry dalla mappa
    icl_hash_delete(storage, fileToRemove, free, NULL);
    
    // Adesso che ho chiamato icl_hash_delete posso liberare anche i due puntatori presi precedentemente.
    free(filePath);

    // Rilascio la lock
    UNLOCK(lock);

    // Libero la memoria della lock
    free(lock);
    
    return 0;
}

// Restituisce il File che ha come filePath quello passato per parametro
File *get_file(char *filePath) {

    // Itero le entry della mappa
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL && strcmp(file->filePath, filePath) == 0) {
                return file;
            }
            curr = curr->next;
        }
    }

    return NULL;
}

// Stampa a schermo le entry della mappa in modalità di debug
void print_storage_debug() {

    DEBUG(("==============================\n"));
    DEBUG(("FILE AMOUNT %ld\n", CURRENT_FILE_AMOUNT));
    DEBUG(("STORAGE SIZE %ld\n\n", CURRENT_STORAGE_SIZE));
    
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                DEBUG(("Path: %s\n", file->filePath));
                DEBUG(("Content: %s\n", file->fileContent));
                DEBUG(("Size: %ld\n", *((size_t *) file->fileSize)));
                DEBUG(("Fifo: %d\n", *((int *) file->fifo)));
                DEBUG(("UsersList:\n"));
                for (Node *usersList = curr->data; usersList != NULL; usersList = usersList->next) {
                    DEBUG(("%d \n", *((int *) usersList->value)));
                }
                DEBUG(("\n"));
            }
            curr = curr->next;
        }
    }
    DEBUG(("==============================\n\n"));
}

// Stampa a schermo le entry della mappa
void print_storage() {
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                printf("SERVER: %s\n", file->filePath);
            }
            curr = curr->next;
        }
    }
    DEBUG(("==============================\n\n"));
}

// Stampa le statistiche richieste
void print_stats() {
    printf("\n");
    printf("SERVER: Numero massimo di file memorizzati: %d\n", TIME_FIFO);
    printf("SERVER: Dimensione massima in MBytes raggiunta: %f MB\n", (float) STORAGE_SIZE_MAX / 1024 / 1024);
    printf("SERVER: Numero di volte in cui l'algoritmo di rimpiazzamento è stato eseguito: %d\n", REPLACEMENT_FREQUENCY);
    printf("SERVER: File contenuti nello storage:\n");
    print_storage();
}

// Libera la memoria della mappa
void destroy_storage() {

    print_storage_debug();
    print_stats();

    // Prendo la lock globale sulla mappa
    LOCK(&STORAGE_LOCK);

    // Itero ogni entry della mappa e rimuovo il file
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                curr = curr->next;
                // Rimuovo la entry
                remove_file_storage(file);
            }
        }
    }

    // Libero la memoria della mappa
    icl_hash_destroy(storage, free, free);

    // Rilascio la lock
    UNLOCK(&STORAGE_LOCK);
}

// Richiesta di apertura del file
int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock) { 
    
    // Cerco il file all'interno dello storage
    File *file = get_file(filePath);

    // Se il file richiesto non esiste e l'utente non ha settato il flag di O_CREATE a 1, restituisco un messaggio di errore
    if (file == NULL && flagCreate == 0) {
        print_storage_debug();
        return -2;
    }

    // Se l'utente sta cercando di creare un file già esistente nella mappa, restituisco un messaggio di errore
    if (file != NULL && flagCreate == 1) {
        print_storage_debug();
        return -2;
    }

    // Se il file non esiste, allora inserisco un nuovo file all'interno dello storage
    if (file == NULL) {
        insert_file_storage(fileDescriptor, filePath);

        print_storage_debug();
        return 0;
    }

    // Se il file esiste, acquisco la lock sul file
    LOCK(file->fileLock);

    // Controllo la lista di utenti che hanno effettuato la open sul file
    Node **usersList = (Node **) icl_hash_find_pointer(storage, file);

    // Se l'utente ha già effettuato la open
    if (contains(*usersList, &fileDescriptor)) {
        // Lascio la lock e restituisco un messaggio di errore
        UNLOCK(file->fileLock);
        
        print_storage_debug();
        return -1;
    }

    // Altrimenti aggiungo il file descriptor dell'utente alla lista
    int *fd;
    if ((fd = (int *) malloc(sizeof(int))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }   

    *fd = fileDescriptor;
    add_tail(usersList, fd);

    // Rilascio la lock del file
    UNLOCK(file->fileLock);

    print_storage_debug();

    return 0;
}

// Richiesta di lettura di un file
void *read_file(int fileDescriptor, char *filePath, int *bufferSize) {

    // Cerco il file all'interno dello storage
    File *file = get_file(filePath);

    // Se il file non esiste restituisco NULL
    if (file == NULL) {
        return NULL;
    }

    // Acquisisco la lock sul file
    LOCK(file->fileLock);
    
    // Se l'utente non ha fatto la open sul file
    Node *usersList = icl_hash_find(storage, file);
    if (!contains(usersList, &fileDescriptor)) {
        // Rilascio la lock e restituisco NULL
        UNLOCK(file->fileLock);
        return NULL;
    }

    // Altrimenti mi calcolo la dimensione del buffer
    *bufferSize = sizeof(char) * strlen(file->fileContent) + 1;

    // Creo il buffer che conterrà il contenuto del file
    char *content;
    if ((content = (char *) malloc(*bufferSize)) == NULL) {
        perror("ERRORE: Impossibile allocare memoria.");
        exit(errno);
    }
    // Copio il contenuto del file nel buffer
    strncpy(content, file->fileContent, *bufferSize);

    // Rilascio la lock del file
    UNLOCK(file->fileLock);

    print_storage_debug();

    // Restituisco il buffer creato
    return content;
}

// Richiesta di lettura di N file
char *read_n_file(int nFiles, int *bufferSize) {

    // Se nFiles è <= 0 o è più grande del numero di file caricati attualmente sul server 
    LOCK(&CAPACITY_LOCK);
    if (nFiles > CURRENT_FILE_AMOUNT || nFiles <= 0) {
        // nFiles prende il numero attuale di file caricati sul server
        nFiles = CURRENT_FILE_AMOUNT;
    }
    UNLOCK(&CAPACITY_LOCK);

    // Se il numero di file caricati corrisponde a 0
    if (nFiles == 0) {
        // Restituisco NULL, non posso leggere nessun file
        return NULL;
    }

    // Prendo la lock globale su tutta la mappa, così gli altri Thread non possono creare nuovi file
    LOCK(&STORAGE_LOCK);

    int pathLength = 0;
    int contentLength = 0;
    int cont = 0;

    // Itero la mappa e calcolo la dimensione totale del buffer che voglio inviare
    for (int i = 0; i < storage->nbuckets && cont < nFiles; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL && cont < nFiles; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                // Acquisisco la lock sul file
                LOCK(file->fileLock);
                // Calcolo la lunghezza del filePath
                pathLength += strlen(file->filePath) + 1;
                // Calcolo la lunchezza del fileContent
                contentLength += strlen(file->fileContent) + 1;
                // Aggiorno la dimensione di bufferSize con il valore appena calcolato
                *bufferSize += pathLength + contentLength + (sizeof(int) * 2);

                cont++;
            }
            curr = curr->next;
        }
    }
    *bufferSize += sizeof(int);

    // ALloco un buffer della dimensione appena calcolata
    char *buffer;
    if ((buffer = (char *) malloc(*bufferSize)) == NULL) {
        perror("ERRORE: Impossibile allocare memoria.");
        exit(errno);
    }

    // Resetto il buffer
    memset(buffer, 0, *bufferSize);
    char *currentBuffer = buffer;

    // Inserisco nel buffer il numero di file che sto per inviare
    memcpy(buffer, &nFiles, sizeof(int));
    buffer += sizeof(int);

    cont = 0;
    pathLength = 0;
    contentLength = 0;

    // Itero la mappa
    for (int i = 0; i < storage->nbuckets && cont < nFiles; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL && cont < nFiles; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                
                // Inserisco nel buffer la lunghezza del filePath
                pathLength = strlen(file->filePath) + 1;
                memcpy(buffer, &pathLength, sizeof(int));
                buffer += sizeof(int);

                // Inserisco nel buffer il filePath
                memcpy(buffer, file->filePath, pathLength);
                buffer += pathLength;

                // Inserisco nel buffer la lunghezza del fileContent
                contentLength = strlen(file->fileContent) + 1;
                memcpy(buffer, &contentLength, sizeof(int));
                buffer += sizeof(int);

                // Inserisco nel buffer fileContent
                memcpy(buffer, file->fileContent, contentLength);
                buffer += contentLength;

                // Rilascio la lock
                UNLOCK(file->fileLock);
                
                cont++;
            }
            curr = curr->next;
        }
    }
    
    // Rilascio la lock globale sulla mappa
    UNLOCK(&STORAGE_LOCK);

    return currentBuffer;
}

// Richiesta di scrittura di un file presente nello storage
int write_file(int fileDescriptor, char *filePath, char *fileContent) {

    // Cerco il file all'interno dello storage
    File *file = get_file(filePath);

    // Se il file non esiste restituisco un messaggio di errore
    if (file == NULL) {
        return -1;
    }

    // Altrimenti prendo la lock sul file
    LOCK(file->fileLock);

    // Se l'utente non ha fatto la open su quel file
    Node *usersList = icl_hash_find(storage, file);
    if (!contains(usersList, &fileDescriptor)) {
        // Rilascio la lock
        UNLOCK(file->fileLock);
        // Restituisco un messaggio di errore
        return -1;
    }

    // Rimuovo il peso del file che sto per modificare dalla capacità dello storage
    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE -= *(file->fileSize);
    UNLOCK(&CAPACITY_LOCK);

    // Mi calcolo la nuova dimensione del file
    size_t fileSize = strlen(file->filePath) + 1 + strlen(fileContent) + 1;    
    
    // Se la dimensione corrente + quella del file che voglio modificare supera la capacità massima, allora applico la politica di rimpiazzamtno
    LOCK(&CAPACITY_LOCK);
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: È stata raggiunta la dimensione massima dello Storage.\n");
        // Se la politica di rimpiazzamento non è andata a buon fine
        if (replace_file_storage(filePath) == -1) {
            
            // Aggiorno la capacità dello storage rimettendo il peso del file che volevo modificare
            LOCK(&CAPACITY_LOCK);
            CURRENT_STORAGE_SIZE += *(file->fileSize);
            if (CURRENT_STORAGE_SIZE > STORAGE_SIZE_MAX) STORAGE_SIZE_MAX = CURRENT_STORAGE_SIZE;
            UNLOCK(&CAPACITY_LOCK);

            // Rilascio la lock sul file
            UNLOCK(file->fileLock);
            
            // Restituisco un messaggio di errore e non modifico più il file, lo lascio come era prima della richiesta
            return -1;
        }
        LOCK(&CAPACITY_LOCK);
    }         
    UNLOCK(&CAPACITY_LOCK);
    
    // Cancello il vecchio peso e il vecchio contenuto del file
    free(file->fileContent);
    free(file->fileSize);

    // Inserisco il nuovo contenuto nel file
    char *content;
    if ((content = (char *) malloc(sizeof(char) * strlen(fileContent) + 1)) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }
    strncpy(content, fileContent, strlen(fileContent) + 1);
    file->fileContent = content;

    // Inserisco il nuovo peso del file
    size_t *size;
    if ((size = (size_t *) malloc(sizeof(size_t))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        exit(errno);
    }
        
    *size = fileSize;
    file->fileSize = size;

    // Aggiorno lo storage con il nuovo peso del file
    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE += *(file->fileSize);
    if (CURRENT_STORAGE_SIZE > STORAGE_SIZE_MAX) STORAGE_SIZE_MAX = CURRENT_STORAGE_SIZE;
    UNLOCK(&CAPACITY_LOCK);

    // Rilascio la lock sul file
    UNLOCK(file->fileLock);

    print_storage_debug();

    return 0;
}

// Richiesta di chiusura di un file
int close_file(int fileDescriptor, char *filePath) {

    // Controllo se il file esiste all'interno dello storage
    File *file = get_file(filePath);

    // Se il file non esiste, restituisco un errore
    if (file == NULL) {
        return -1;
    }

    // Altrimenti prendo la lock sul file
    LOCK(file->fileLock);

    // Prendo la lista di utenti che hanno eseguito la open su quel file
    Node **usersList = (Node **) icl_hash_find_pointer(storage, file);    

    // Se l'utente non ha eseguito la open
    int *fd;
    if ((fd = get_value(*usersList, &fileDescriptor)) == NULL) {
        // Rilascio la lock e restituisco un mesaggio di errore
        UNLOCK(file->fileLock);
        return -1;
    }

    // Altrimenti rimuovo il valore dalla lista e libero la memoria
    remove_value(usersList, fd);
    free(fd);
    
    // Rilascio la lock
    UNLOCK(file->fileLock);

    print_storage_debug();

    return 0;
}

// Richiesta di rimozione di un file dallo storage
int remove_file(int fileDescriptor, char *filePath) {
    
    // Cerco il file all'interno dello storage
    File *file = get_file(filePath);

    // Se il file non esiste, allora inserisco un nuovo file all'interno dello storage
    if (file == NULL) {
        return -1;
    }

    // Se l'utente non ha eseguito la open sul file
    Node *usersList = icl_hash_find(storage, file);
    if (!contains(usersList, &fileDescriptor)) {
        // Rilascio la lock
        UNLOCK(file->fileLock);
        // Restituisco un messaggio di errore
        return -1;
    }

    // Altrimenti rimuovo il file dallo storage
    return remove_file_storage(file);
}

// Metodo che gestisce la disconnessione di un client
void disconnect_client(int fileDescriptor) {

    printf("SERVER: Disconnessione dell'utente %d\n", fileDescriptor);
    
    // Itero tutta la mappa
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                // Prendo la lock sul file
                LOCK(file->fileLock);
                // Prendo la lista di utenti che hanno fatto la open su quel file
                Node **usersList = (Node **) &(curr->data);
                // Prendo il puntatore del fd dell'utente disconnesso
                int *fd = get_value(*usersList, &fileDescriptor);
                
                // Se l'utente disconnesso aveva fatto la open sul file che sto iterando
                if (fd != NULL) {
                    // Rimuovo il nodo del fd
                    remove_value(usersList, &fileDescriptor);
                    // Libero la memoria del fd
                    free(fd);   
                }

                // Rilascio la lock del file
                UNLOCK(file->fileLock);
            }
            curr = curr->next;
        }
    }

    print_storage_debug();
}

