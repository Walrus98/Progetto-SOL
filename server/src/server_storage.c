#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "../include/server_storage.h"
#include "../include/server_cache_handler.h"
#include "../include/icl_hash.h"
#include "../include/pthread_utils.h"

/**
 * Lo storage è formato da due strutture dati:
 *  - una cache, definita come una lista, essa cambia comportamento e contenuto in base al tipo di politica di rimpiazzamento adottata.
 *  Questa lista contiene tutti i File caricati in memoria dagli utenti. La struct del file è definita in @see server_cache_handler.h
 * 
 *  - una mappa, chiamata clientFiles, che viene utilizzata per vedere su quali file gli utenti hanno eseguito una open. La mappa è formata da:
 *      - come chiave, il fileDescriptor che rappresenta in maniera univoca l'utente (fino alla disconnessione)
 *      - come valore, una lista di struct di FileOpened.
 *      La struct di FileOpened è formata a sua volta da:
 *          - Un intero per vedere se l'utente ha settato la lock su quel file
 *          - Un char* che contiene il nome del file
 *
 * Attraverso questa mappa, quindi, è possibile vedere se l'utente ha eseguito o meno la open sul file. Inoltre, in caso di disconnessione, è possibile
 * vedere su quali file il client aveva settato ad 1 il Flag lock, così da poterlo resettare per rendere nuovamente disponibile il file agli utenti.
 **/

// Capacità massima del server storage
static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

// Capacità corrente del server storage
static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

static icl_hash_t *clientFiles;
static pthread_mutex_t clientFilesMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct FileOpened {
    int *lock;
    char *path;
} FileOpened;

// Metodi della storage formato da mappa e lista
void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy);
void insert_storage(File file);
void print_storage();
void destroy_storage();

// Metodi della mappa
int add_client_files(int fileDescriptor, char *filePath, int flagLock);
FileOpened *get_client_files(Node *fdList, char *path);
void remove_client_files(Node **fdList, char *filePath);
void print_client_files();

// Metodi per gestire le richieste inviate dai client
int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock);
void *read_file(int fileDescriptor, char *filePath, int *bufferSize);
char *read_n_file(int nFiles, int *bufferSize);
int write_file(int fileDescriptor, char *filePath,  char *fileContent);
int close_file(int fileDescriptor, char *filePath);

// ========================= METODI DELLO STORAGE =========================

// Inizializza lo storage, viene chiamato all'avvio del programma
void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy) {
    // Imposto il limite dello storage
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    // Stabilisco il tipo di politica di rimpiazzamento in base a quella passata dal file config 
    inizialize_policy(replacementPolicy);

    // Creo una mappa che viene utilizzata per vedere quali su quali file gli utenti hanno eseguito la open  
    clientFiles = icl_hash_create(100, NULL, int_compare);
}

// Inserisco un file all'interno della cache
void insert_storage(File file) {

    // Controllo se il file che sto inserendo è già presente nella cache
    File *tempFile = get_file_cache(file.filePath);

    // Se è presente, significa che devo aggiornare il contenuto del file
    if (tempFile != NULL) {       
        // Rimuovo momentaneamente il file dalla cache
        remove_file_cache(tempFile->filePath);

        // Aggiorno la dimensione attuale dello storage
        CURRENT_FILE_AMOUNT--;
        CURRENT_STORAGE_SIZE -= *(tempFile->fileSize);
    }

    // Se il file da inserire supera la capacità massima dello storage, annullo l'inserimento
    if (*(file.fileSize) > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: il file non è stato inserito in quanto supera la capacità massima dello Storage.\n");
        return;
    }
    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta il massimo numero di File nello Storage.\n");
        
        // Prendo il file da rimuovere in base alla politica di rimpiazzamento adottata
        File *fileToRemove = replacement_file_cache();

        // Aggiorno la dimensione attuale dello storage
        CURRENT_STORAGE_SIZE -= *(fileToRemove->fileSize);
        CURRENT_FILE_AMOUNT--;

        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);        
    
        // Rimuovo il file dalla memoria principale
        free(fileToRemove->filePath);
        free(fileToRemove->fileContent);
        free(fileToRemove->fileSize);
        free(fileToRemove->fileLocked);
        free(fileToRemove->fileOpens);
        free(fileToRemove);
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    while (CURRENT_STORAGE_SIZE + *(file.fileSize) > STORAGE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta la dimensione massima dello Storage.\n");
        
        // Prendo il file da rimuovere in base alla politica di rimpiazzamento adottata
        File *fileToRemove = replacement_file_cache();
        
        // Aggiorno la dimensione attuale dello storage
        CURRENT_STORAGE_SIZE -= *(fileToRemove->fileSize);
        CURRENT_FILE_AMOUNT--;

        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);

        // Rimuovo il file dalla memoria principale
        free(fileToRemove->filePath);
        free(fileToRemove->fileContent);
		free(fileToRemove->fileSize);
        free(fileToRemove->fileLocked);
        free(fileToRemove->fileOpens);
        free(fileToRemove);
    }

    // Inserisco il nuovo file o il file aggiornato all'interno della cache
    insert_file_cache(file);

    // Se il file era già presente nella lista e quindi è stato aggiornato, faccio la free del vecchio file
    if (tempFile != NULL) {
        free(tempFile->filePath);
        free(tempFile->fileContent);
        free(tempFile->fileSize);
        free(tempFile->fileLocked);
        free(tempFile->fileOpens);
        free(tempFile);
    }

    // Aggiorno la dimensione attuale dello storage
    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += *(file.fileSize);
}

// Stampa la mappa e la lista
void print_storage() {
    print_client_files();
    print_cache();
}

// Viene chiamato dal server al momento dello spegnimento. Rimuove dalla memoria principale i file e tutti gli elementi contenuti nella mappa e nella lista
void destroy_storage() {

    // Acquisico la Lock sulla mappa
    LOCK(&clientFilesMutex);

    icl_entry_t *bucket, *curr;

    // Itero tutta la mappa e cancello il fileOpened
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    FileOpened *fileOpened = (FileOpened *) temp->value;
                    free(fileOpened->path);
                    free(fileOpened->lock);
                    free(fileOpened);
                }
            }
            curr = curr->next;
        }
    }

    // Itero tutta la mappa e cancello i nodi della lista
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    Node *freeNode = curr->data;
                    curr = curr->next;
                    free(freeNode);
                }
            }
            curr = curr->next;
        }
    }
  
    // Cancello tutte le chiavi della mappa
    icl_hash_destroy(clientFiles, free, NULL);
    
    // Rilascio la lock
    UNLOCK(&clientFilesMutex);

    // Rimuovo dalla memoria principale i file e i nodi della lista
    destroy_cache();
}

// ========================= METODI DELLA MAPPA =========================

// Aggiunge un FileOpened nella lista corrispondente alla chiave del fileDescriptor
int add_client_files(int fileDescriptor, char *filePath, int flagLock) {
    
    // Acquisico la lock sulla mappa
    LOCK(&clientFilesMutex);
   
    // Alloco nello Heap la memoria necessaria per un FileOpened
    FileOpened *fileOpened = (FileOpened *) malloc(sizeof(FileOpened));

    // Alloco nello Heap il valore di lock del file
    int *lock = (int *) malloc(sizeof(int));
    *lock = flagLock;
    
    // Alloco nello Heap il filePath
    char *path = malloc(sizeof(char) * (strlen(filePath) + 1));
    strcpy(path, filePath);

    // Inizializzo le due variabili
    fileOpened->lock = lock;
    fileOpened->path = path;

    // Cerco la lista del file descriptor passato per parametro
    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // Se la lista è NULL, significa che l'utente sta facendo la sua prima open
    if (fdList == NULL) {      
        // Quindi inizializzo la lista dell'utente
        Node *newFdList = NULL;

        // Inserisco nella lista appena creata il FileOpened
        add_tail(&newFdList, fileOpened);

        // ALloco nell'heap il file descriptor dell'utente
        int *fd = (int *) malloc(sizeof(int));
        // Assegno alla variabile il file descriptor dell'utente passato per parametro
        *fd = fileDescriptor;

        // Inserisco un entry nella mappa che ha come chiave, il fd dell'utente e come valore la lista di FileOpened
        icl_hash_insert(clientFiles, fd, newFdList);

    // Se la lista è diversa da NULL significa che l'utente ha eseguito almeno una open e quindi la sua entry è già presente nella mappa,
    // per prima cosa controllo che il client non abbia richiesto una open su un file che aveva precedentemente aperto
    } else if (get_client_files(fdList, fileOpened->path) == NULL) {
        // Se il file non è presente, allora lo aggiungo il FileOpened alla lista
        add_tail(&fdList, fileOpened);
    // Se il client ha richiesto la open su un file che aveva già aperto precedentemente
    } else {
        // Libero la memoria allocata per creare il FileOpened
        free(path);
        free(lock);
        free(fileOpened);

        // Rilascio la lock
        UNLOCK(&clientFilesMutex);
        
        // Restituisco un messaggio di errore
        return -1;
    }
    
    // Rilascio la Lock
    UNLOCK(&clientFilesMutex);
    
    print_storage();
    
    // L'operazione è andata a buon fine
    return 1;
}

// Rimuove un FileOpened dalla lista corrispondente alla chiave del fileDescriptor
void remove_client_files(Node **fdList, char *filePath) {

    // Rimuovo il nodo dalla lista della mappa
    if (*fdList != NULL) {
		Node *currentFdList = *fdList;
		Node *precFdList = NULL;

		while (currentFdList != NULL) {
            FileOpened *fileOpened = (FileOpened *) currentFdList->value;
			if (strncmp(fileOpened->path, filePath, STRING_SIZE) == 0) {
				if (precFdList == NULL) {
					Node *tempNode = *fdList;
                    *fdList = (*fdList)->next;
					free(tempNode);  
					return;
				} else {
					Node *tempNode = *fdList;
					precFdList->next = currentFdList->next;
					free(tempNode);
					return;
				}
			}
			precFdList = currentFdList;
			currentFdList = currentFdList->next;
		}
	}
}

// Restituisce il FileOpened che ha come filePath quello corrispondente alla variaible passata per parametro
FileOpened *get_client_files(Node *fdList, char *path) {
    // Itero la lista di FileOpened
	for (; fdList != NULL; fdList = fdList->next) {
        // Prendo il valore dalla nodo della lista
		FileOpened *fileOpened = (FileOpened *) fdList->value;   
        // Se il nome di fileOpened corrisponde a quello passato per parametro     
        if (strncmp(fileOpened->path, path, STRING_SIZE) == 0) {
            // Restituisco il file
			return fileOpened;
		}
	}
    // Altrimenti restituisco NULL
	return NULL;
}

// Stampa la struttura dati della mappa
void print_client_files() {
    
    icl_entry_t *bucket, *curr;

    printf("==== Client Files Map ====\n");
    
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                int *fd = curr->key;
                printf("%d -> ", *fd);
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    FileOpened *fileOpened = (FileOpened *) temp->value;
                    printf("[%s, %d] ", fileOpened->path, *(fileOpened->lock));
                }
                printf("\n");
            }
            curr = curr->next;
        }
    }

    printf("\n");    
}

// ========================= METODI CHE GESTISOCNO LE RICHIESTE DEI CLIENT =========================

// Metodo che viene eseguito quando un client richiede una open di un file al Server
int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock) {

    // Prendo il file dalla cache
    File *file = get_file_cache(filePath);

    // Controllo i casi in cui il server deve dare errore
    if ((flagCreate == 1 && file != NULL) || (flagCreate == 0 && file == NULL)) {
        return 0;
    }

    // Se il client ha settato il flag Create a 1 e il file non esiste
    if (flagCreate == 1 && file == NULL) {

        // Creo un nuovo file momentaneamente nello stack
        File newFile;
        // Assegno al file il filePath passato per argomento
        newFile.filePath = filePath;
        // All'inizio il file non avrà contenuto, verrà inserito dopo tramite una write o un append
        newFile.fileContent = "";
        // Calcolo il peso iniziale del file
        size_t size = get_file_size(newFile);
        // Setto al file la dimensione calcolata
        newFile.fileSize = &size;
        // Setto lo stato di lock con quello passato per argomento
        newFile.fileLocked = &flagLock;
        // Assegno il numero di open effettuate sul file a 1
        int opens = 1;
        newFile.fileOpens = &opens;

        // Inserisco il file all'interno della cache
        insert_storage(newFile); 

        // Inserisco il file all'interno della mappa 
        add_client_files(fileDescriptor, newFile.filePath, flagLock);   

        return 1;
    }

    // Se il client ha settato il flag Create a 0 e il file esiste
    if (flagCreate == 0 && file != NULL) {

        // Controllo in mutua esclusione lo stato di lock del file
        int locked = get_file_lock(file);

        // Se il file di lock è settato a 1, allora non posso fare la open
        if (locked == 1) {
            // Restituisco un messaggio di errore
            return -2;
        }

        // Se l'utente vuole aprire il file in stato di lock ma attualmente è giò aperto da altri
        // utenti che non hanno settato il file lock a 1
        if (flagLock == 1 && get_files_opens(file) != 0) {
            // Restituisco un messaggio di errore
            return -3;
        }

        // Modifico in mutua esclusione il valore di Lock contenuto nel file 
        if (locked != flagLock) {
            set_file_lock(file, flagLock);
        }

        // Segno nella mappa che l'utente ha aperto il file passato per parametro
        int response = add_client_files(fileDescriptor, file->filePath, flagLock);

        // Se l'utente non ha fatto la open su un file che aveva aperto precedentemente, e quindi l'operazione
        // è andata a buon fine
        if (response == 1) {                
            // Incremento il numero di open effettuate su quel file in mutua esclusione
            increase_file_opens(file);
        }

        return response;
    }

    return 0;
}

// Metodo che viene eseguito quando un client richiede una read di un file al Server
void *read_file(int fileDescriptor, char *filePath, int *bufferSize) {
    
    // Prendo la lock sulla mappa
    LOCK(&clientFilesMutex);

    // Prendo la lista di file aperti da quel client
    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // Creo un buffer che conterrà il messaggio da restituire al client
    char *payload = NULL;

    // Se l'utente ha fatto la open sul file, allora può eseguire la read
    if (get_client_files(fdList, filePath) != NULL) {

        // Prendo in mutua esclusione il file dalla cache
        File *file = get_file_cache(filePath);

        // Se file è uguale a NULL, significa che l'utente ha eseguto correttamente la open, ma il file è stato cancellato dalla politica di rimpiazzamento
        // if (file != NULL) {

            // Calcolo la lunghezza del contenuto del file
            int contentLength = strlen(file->fileContent) + 1;
            // Inizializzo il buffer con la dimensione necessaria per contenere il contenuto del file
            payload = malloc(contentLength);
            // Copio nel buffer il contenuto del file
            memcpy(payload, file->fileContent, contentLength);
            
            // Modifico la variabile per riferimento passandoci la dimensione del buffer
            *bufferSize = contentLength;
        // } 
    }
    
    // Rilascio la lock della mappa
    UNLOCK(&clientFilesMutex);
    
    print_storage();
    
    // Restituisco il buffer, se l'utente non ha fatto la open su quel file restituisco NULL
    return payload;
}

// Metodo che viene eseguito quando un client richiede una read_n al Server
char *read_n_file(int nFiles, int *bufferSize) {

    // Se il numero di file richiesti è minore di 0 oppure i file attualmente
    // caricati sul server sono minori del numero di file richiesti
    if (nFiles <= 0 || nFiles > CURRENT_FILE_AMOUNT) {
        // Allora chiamo la funzione passandogli come paramentro il numero di file attualmente caricati sul server
        return get_n_files_cache(CURRENT_FILE_AMOUNT, bufferSize);
    }
    
    // Altrimenti leggo nFiles richiesti
    return get_n_files_cache(nFiles, bufferSize);
}

int write_file(int fileDescriptor, char *filePath, char *fileContent) {
    
    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    int response = 0;

    // Se l'utente ha fatto la open sul file
    if (get_client_files(fdList, filePath) != NULL) {
        File *file = get_file_cache(filePath);

        File tempFile;
        tempFile.filePath = file->filePath;
        tempFile.fileContent = fileContent;
        tempFile.fileLocked = file->fileLocked;
        tempFile.fileOpens = file->fileOpens;
        size_t size = get_file_size(tempFile);
        tempFile.fileSize = &size;

        insert_storage(tempFile); 

        response = 1;    
    }
    
    print_storage();

    UNLOCK(&clientFilesMutex);

    return response;
}

int remove_file(int fileDescriptor, char *filePath) {

    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    int response = 0;

    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;
        if (strncmp(fileOpened->path, filePath, STRING_SIZE) == 0) {
            if (*(fileOpened->lock) == 1) {

                File *file = get_file_cache(fileOpened->path);

                CURRENT_STORAGE_SIZE -= *(file->fileSize);
                CURRENT_FILE_AMOUNT--;

                remove_file_cache(file->filePath);
                free(file->filePath);
                free(file->fileContent);
                free(file->fileSize);
                free(file->fileLocked);
                free(file->fileOpens);
                free(file);

                remove_client_files(&fdList, fileOpened->path);
                free(fileOpened->path);
                free(fileOpened->lock);
                free(fileOpened);
                
                response = 1;                
            } else {
                response = -1;
            }
            break;
        }
    }

    // Se la lista della mappa è uguale a null, significa che l'utente ha fatto la close sull'ultimo file che aveva aperto, quindi devo
    // cancellare la chiave dalla mappa e quando andrà ad eseguire una nuova open, riallocherò un nuovo fd. Altrimenti la insert alloca 4byte di memoria in più
    if (fdList == NULL) {
        icl_hash_delete(clientFiles, &fileDescriptor, free, NULL);
    }
    
    print_storage();
    
    UNLOCK(&clientFilesMutex);    

    return response;
}

int close_file(int fileDescriptor, char *filePath) {
    
    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    int response = 0;

    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;
        if (strncmp(fileOpened->path, filePath, STRING_SIZE) == 0) {
            File *file = get_file_cache(fileOpened->path);

            if (*(fileOpened->lock) == 1) {
                set_file_lock(file, 0);
            }
            decrease_file_opens(file);         

            remove_client_files(&fdList, fileOpened->path);
            free(fileOpened->path);
            free(fileOpened->lock);
            free(fileOpened);

            response = 1;

            break;
        }
    }

    // Se la lista della mappa è uguale a null, significa che l'utente ha fatto la close sull'ultimo file che aveva aperto, quindi devo
    // cancellare la chiave dalla mappa e quando andrà ad eseguire una nuova open, riallocherò un nuovo fd. Altrimenti la insert alloca 4byte di memoria in più
    if (fdList == NULL) {        
        icl_hash_delete(clientFiles, &fileDescriptor, free, NULL);
    }
    
    print_storage();
    
    UNLOCK(&clientFilesMutex);    

    return response;
}

// Gestisco la disconnessione di un client
void disconnect_client(int fileDescriptor) {

    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // cancello il valore associato ai nodi
    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;

        File *file = get_file_cache(fileOpened->path);

        if (*(fileOpened->lock) == 1) {
            set_file_lock(file, 0);
        }
        decrease_file_opens(file);

        free(fileOpened->path);
        free(fileOpened->lock);
        free(fileOpened);
    }

    // cancello i nodi
    for (Node *curr = fdList; curr != NULL; ) {
        Node *temp = curr;
        curr = curr->next;
        free(temp);
    }

    // cancello la chiavi
    icl_hash_delete(clientFiles, &fileDescriptor, free, NULL);
    
    UNLOCK(&clientFilesMutex);
    
    print_storage();
}
