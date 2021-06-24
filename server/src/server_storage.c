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
static int TIME_FIFO = 0;
static size_t STORAGE_SIZE_MAX = 0;
static int REPLACEMENT_FREQUENCY = 0;

static icl_hash_t *storage;
static pthread_mutex_t STORAGE_LOCK = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t CAPACITY_LOCK = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t REPLACEMENT_FREQUENCY_LOCK = PTHREAD_MUTEX_INITIALIZER;

// Funzione di Hash utilizzata con la mappa.
#define BITS_IN_int     ( sizeof(int) * CHAR_BIT )
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       ( ~((unsigned int)(~0) >> ONE_EIGHTH ))

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

int hash_value(void *a, void *b) {

    File *file = (File *) a;
    // char *filePath = (char *) b;
    File *file2 = (File *) b;

    int res = strcmp(file->filePath, file2->filePath);
    return res == 0 ? 1 : 0;
}

int list_compare(void *a, void *b) {
    return *((int *) a) == *((int *) b);
}

// Inizalizza la Mappa
void create_storage(size_t fileCapacity, size_t storageCapacity);
// Inserisce un file all'interno dello storage
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

int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock);
void *read_file(int fileDescriptor, char *filePath, int *bufferSize);
char *read_n_file(int nFiles, int *bufferSize);
int write_file(int fileDescriptor, char *filePath, char *fileContent);
int close_file(int fileDescriptor, char *filePath);
int remove_file(int fileDescriptor, char *filePath);
void disconnect_client(int fileDescriptor);

void create_storage(size_t fileCapacity, size_t storageCapacity) {
    // Imposto il limite dello storage
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    // Creo una mappa che viene utilizzata per vedere quali su quali file gli utenti hanno eseguito la open  
    storage = icl_hash_create(fileCapacity, hash_key, hash_value);
}

int insert_file_storage(int fileDescriptor, char *filePath) {

    LOCK(&STORAGE_LOCK);

    size_t pathLength = strlen(filePath) + 1;
    size_t contentLength = strlen("") + 1;
    size_t fileSize = pathLength + contentLength;

    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    LOCK(&CAPACITY_LOCK);
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: È stato raggiunto il massimo numero di File nello Storage.\n");
        replace_file_storage(filePath);
    }
    UNLOCK(&CAPACITY_LOCK);

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    LOCK(&CAPACITY_LOCK);
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: È stata raggiunta la dimensione massima dello Storage.\n");
        if (replace_file_storage(filePath) == -1) {
            // UNLOCK(&CAPACITY_LOCK);
            UNLOCK(&STORAGE_LOCK)
            return -1;
        }
        LOCK(&CAPACITY_LOCK);
    }     
    UNLOCK(&CAPACITY_LOCK);

    File *newFile = (File *) malloc(sizeof(File));
   
	char *path = (char *) malloc(pathLength);
	strncpy(path, filePath, pathLength);
	newFile->filePath = path;

    char *content = (char *) malloc(contentLength);
	strncpy(content, "", contentLength);
	newFile->fileContent = content;

    size_t *size = (size_t *) malloc(sizeof(size_t));
	*size = fileSize;
	newFile->fileSize = size;

    int *fifo = (int *) malloc(sizeof(int));
    
    *fifo = TIME_FIFO;
    TIME_FIFO++;

    newFile->fifo = fifo;

    pthread_mutex_t *lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lock, NULL);
	newFile->fileLock = lock;

    Node *usersList;
    create_list(&usersList, list_compare);
    int *fd = (int *) malloc(sizeof(int));
    *fd = fileDescriptor;
    add_tail(&usersList, fd);
        
    icl_hash_insert(storage, newFile, usersList);
    
    LOCK(&CAPACITY_LOCK);
    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += *(newFile->fileSize);
    if (CURRENT_STORAGE_SIZE > STORAGE_SIZE_MAX) STORAGE_SIZE_MAX = CURRENT_STORAGE_SIZE;
    UNLOCK(&CAPACITY_LOCK);
    
    UNLOCK(&STORAGE_LOCK);

    return 0;
}

int replace_file_storage(char *filePath) {

    LOCK(&REPLACEMENT_FREQUENCY_LOCK);
    REPLACEMENT_FREQUENCY++;
    UNLOCK(&REPLACEMENT_FREQUENCY_LOCK);

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

    if (fileToRemove == NULL) {
        fprintf(stderr, "ATTENZIONE: Non è possibile rimuovere nessun altro file dallo storage, quindi la richiesta non viene eseguita!\n");
        return -1;
    }
    
    fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);        

    remove_file_storage(fileToRemove);

    return 0;
}


int remove_file_storage(File *fileToRemove) {

    if (fileToRemove == NULL) {
        return -1;
    }

    LOCK(fileToRemove->fileLock);

    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE -= *(fileToRemove->fileSize);
    CURRENT_FILE_AMOUNT--;
    UNLOCK(&CAPACITY_LOCK);

    Node **usersList = (Node **) icl_hash_find_pointer(storage, fileToRemove);

    for (Node *currentList = *usersList; currentList != NULL; ) {
        Node *temp = currentList;
        currentList = currentList->next;
        
        free(temp->value);
        free(temp);
    }

    pthread_mutex_t *lock = fileToRemove->fileLock;

    char *filePath = fileToRemove->filePath;

    free(fileToRemove->fileContent);
    free(fileToRemove->fileSize);
    free(fileToRemove->fifo);

    icl_hash_delete(storage, fileToRemove, free, NULL);
    
    free(filePath);

    UNLOCK(lock);
    free(lock);
    
    return 0;
}

File *get_file(char *filePath) {

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

void print_stats() {
    printf("\n");
    printf("SERVER: Numero massimo di file memorizzati: %d\n", TIME_FIFO);
    printf("SERVER: Dimensione massima in MBytes raggiunta: %f MB\n", (float) STORAGE_SIZE_MAX / 1000 / 1000);
    printf("SERVER: Numero di volte in cui l'algoritmo di rimpiazzamento è stato eseguito: %d\n", REPLACEMENT_FREQUENCY);
    printf("SERVER: File contenuti nello storage:\n");
    print_storage();
}

void destroy_storage() {

    print_storage_debug();
    print_stats();

    LOCK(&STORAGE_LOCK);

    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                curr = curr->next;
                remove_file_storage(file);
            }
        }
    }

    icl_hash_destroy(storage, free, free);

    UNLOCK(&STORAGE_LOCK);
}


int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock) { 
    
    // Cerco il file all'interno dello storage
    File *file = get_file(filePath);

    if (file == NULL && flagCreate == 0) {
        print_storage_debug();
        return -2;
    }

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

    // Aggiungo il file descriptor dell'utente alla lista
    int *fd = (int *) malloc(sizeof(int));
    *fd = fileDescriptor;
    add_tail(usersList, fd);

    UNLOCK(file->fileLock);

    print_storage_debug();

    return 0;
}

void *read_file(int fileDescriptor, char *filePath, int *bufferSize) {

    File *file = get_file(filePath);

    if (file == NULL) {
        return NULL;
    }

    LOCK(file->fileLock);
    
    Node *usersList = icl_hash_find(storage, file);
    if (!contains(usersList, &fileDescriptor)) {
        UNLOCK(file->fileLock);
        return NULL;
    }

    *bufferSize = sizeof(char) * strlen(file->fileContent) + 1;

    char *content = (char *) malloc(*bufferSize);
    strncpy(content, file->fileContent, *bufferSize);

    UNLOCK(file->fileLock);

    print_storage_debug();

    return content;
}

char *read_n_file(int nFiles, int *bufferSize) {

    LOCK(&CAPACITY_LOCK);
    if (nFiles > CURRENT_FILE_AMOUNT || nFiles <= 0) {
        nFiles = CURRENT_FILE_AMOUNT;
    }
    UNLOCK(&CAPACITY_LOCK);

    if (nFiles == 0) {
        return NULL;
    }

    int pathLength = 0;
    int contentLength = 0;
    int cont = 0;

    for (int i = 0; i < storage->nbuckets && cont < nFiles; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL && cont < nFiles; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                LOCK(file->fileLock);
                pathLength += strlen(file->filePath) + 1;
                contentLength += strlen(file->fileContent) + 1;
                *bufferSize += pathLength + contentLength + (sizeof(int) * 2);
                UNLOCK(file->fileLock);

                cont++;
            }
            curr = curr->next;
        }
    }
    *bufferSize += sizeof(int);

    char *buffer = (char *) malloc(*bufferSize);
    memset(buffer, 0, *bufferSize);

    char *currentBuffer = buffer;

    memcpy(buffer, &nFiles, sizeof(int));
    buffer += sizeof(int);

    cont = 0;
    pathLength = 0;
    contentLength = 0;

    for (int i = 0; i < storage->nbuckets && cont < nFiles; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL && cont < nFiles; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                LOCK(file->fileLock);

                pathLength = strlen(file->filePath) + 1;
                memcpy(buffer, &pathLength, sizeof(int));
                buffer += sizeof(int);

                memcpy(buffer, file->filePath, pathLength);
                buffer += pathLength;

                contentLength = strlen(file->fileContent) + 1;
                memcpy(buffer, &contentLength, sizeof(int));
                buffer += sizeof(int);

                memcpy(buffer, file->fileContent, contentLength);
                buffer += contentLength;

                UNLOCK(file->fileLock);

                cont++;
            }
            curr = curr->next;
        }
    }

    return currentBuffer;
}

int write_file(int fileDescriptor, char *filePath, char *fileContent) {

    File *file = get_file(filePath);

    if (file == NULL) {
        return -1;
    }

    LOCK(file->fileLock);

    Node *usersList = icl_hash_find(storage, file);
    if (!contains(usersList, &fileDescriptor)) {
        UNLOCK(file->fileLock);
        return -1;
    }

    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE -= *(file->fileSize);
    UNLOCK(&CAPACITY_LOCK);

    size_t fileSize = strlen(file->filePath) + 1 + strlen(fileContent) + 1;    
    
    LOCK(&CAPACITY_LOCK);
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: È stata raggiunta la dimensione massima dello Storage.\n");
        if (replace_file_storage(filePath) == -1) {

            LOCK(&CAPACITY_LOCK);
            CURRENT_STORAGE_SIZE += *(file->fileSize);
            if (CURRENT_STORAGE_SIZE > STORAGE_SIZE_MAX) STORAGE_SIZE_MAX = CURRENT_STORAGE_SIZE;
            UNLOCK(&CAPACITY_LOCK);

            UNLOCK(file->fileLock);
            return -1;
        }
        LOCK(&CAPACITY_LOCK);
    }         
    UNLOCK(&CAPACITY_LOCK);
    
    free(file->fileContent);
    free(file->fileSize);

    char *content = malloc(sizeof(char) * strlen(fileContent) + 1);
    strncpy(content, fileContent, strlen(fileContent) + 1);
    file->fileContent = content;

    size_t *size = malloc(sizeof(size_t));
    *size = fileSize;

    file->fileSize = size;

    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE += *(file->fileSize);
    if (CURRENT_STORAGE_SIZE > STORAGE_SIZE_MAX) STORAGE_SIZE_MAX = CURRENT_STORAGE_SIZE;
    UNLOCK(&CAPACITY_LOCK);

    UNLOCK(file->fileLock);

    print_storage_debug();

    return 0;
}


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

int remove_file(int fileDescriptor, char *filePath) {
    
    // Cerco il file all'interno dello storage
    File *file = get_file(filePath);

    // Se il file non esiste, allora inserisco un nuovo file all'interno dello storage
    if (file == NULL) {
        return -1;
    }

    Node *usersList = icl_hash_find(storage, file);
    if (!contains(usersList, &fileDescriptor)) {
        UNLOCK(file->fileLock);
        return -1;
    }

    return remove_file_storage(file);
}

void disconnect_client(int fileDescriptor) {

    printf("SERVER: Disconnessione dell'utente %d\n", fileDescriptor);
    
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                // Prendo la lista
                Node **usersList = (Node **) &(curr->data);
                // Prendo il puntatore al fd dell'utente disconnesso
                int *fd = get_value(*usersList, &fileDescriptor);
                // Rimuovo il nodo del fd
                remove_value(usersList, &fileDescriptor);
                // Libero la memoria
                free(fd);
            }
            curr = curr->next;
        }
    }

    print_storage_debug();
}

