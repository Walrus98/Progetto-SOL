#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "../include/server_storage.h"
#include "../include/server_cache_handler.h"
#include "../include/icl_hash.h"
#include "../include/pthread_utils.h"

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

void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy);
void insert_storage(File file);
void print_storage();
void destroy_storage();

int add_client_files(int fileDescriptor, File file, int flagLock);
void remove_client_files(int fileDescriptor);
int contains_client_files(Node *fdList, char *path);
void print_client_files();

int openFile(int fileDescriptor, char *filePath, int flagCreate, int flagLock);

void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    inizialize_policy(replacementPolicy);

    clientFiles = icl_hash_create(100, NULL, int_compare);
}

int openFile(int fileDescriptor, char *filePath, int flagCreate, int flagLock) {

    File *file = get_file_cache(filePath);

    if ((flagCreate == 1 && file != NULL) || (flagCreate == 0 && file == NULL)) {
        return 0;
    }

    if (flagCreate == 1 && file == NULL) {

        File newFile;
        newFile.filePath = filePath;
        newFile.fileContent = "";
        newFile.fileSize = get_file_size(newFile); 
        newFile.locked = flagLock;

        insert_storage(newFile); 

        return add_client_files(fileDescriptor, newFile, flagLock);   
    }

    if (flagCreate == 0 && file != NULL) {
        if (file->locked == 1) {
            return -2;
        }
        if (flagLock == 1) {
            file->locked = 1;
        }
        return add_client_files(fileDescriptor, *file, flagLock);
    }

    return 0;
}

void insert_storage(File file) {

    // Se il file da inserire supera la capacità massima dello storage, annullo l'inserimento
    if (file.fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: il file non è stato inserito in quanto supera la capacità massima dello Storage.\n");
        return;
    }
    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta il massimo numero di File nello Storage.\n");
        
        File *fileToRemove = replacement_file_cache();

        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    while (CURRENT_STORAGE_SIZE + file.fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta la dimensione massima dello Storage.\n");
        
        File *fileToRemove = replacement_file_cache();

        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);
    }

    insert_file_cache(file);

    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += file.fileSize;
}

void print_storage() {
    print_cache();
}

int add_client_files(int fileDescriptor, File file, int flagLock) {
    
    // Acquisico la lock sulla mappa
    LOCK(&clientFilesMutex);
   
    // Cerco la lista del file descriptor passato per parametro
    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // Alloco la memoria
    FileOpened *fileOpened = (FileOpened *) malloc(sizeof(FileOpened));

    int *lock = (int *) malloc(sizeof(int));
    *lock = flagLock;
    
    char *path = malloc(sizeof(char) * (strlen(file.filePath) + 1));
    strcpy(path, file.filePath);

    fileOpened->lock = lock;
    fileOpened->path = path;

    // Se la lista è NULL, significa che l'utente ha fatto la sua prima open
    if (fdList == NULL) {      
        // Inizializzo la lista dell'utente
        Node *newFdList = NULL;

        // ALloco nell'heap il file descriptor dell'utente
        int *fd = (int *) malloc(sizeof(int));
        // Assegno alla variabile il file descriptor dell'utente passato per parametro
        *fd = fileDescriptor;

        add_tail(&newFdList, fileOpened);

        // Inserisco la prima entry nella mappa che ha come chiave, il fd dell'utente e come valore la lista di file path a cui ha fatto la open
        icl_hash_insert(clientFiles, fd, newFdList);

    // Controllo che il client non abbia richiesto una open su un file già presente nella lista
    } else if (!contains_client_files(fdList, fileOpened->path)) {
        // Se il file non è presente, allora lo aggiungo
        add_tail(&fdList, fileOpened);
    // Se il client ha richiesto la open su un file che aveva già aperto precedentemente
    } else {
        // Libero la memoria allocata
        free(path);
        free(lock);
        free(fileOpened);
        // Rilascio la lock
        UNLOCK(&clientFilesMutex);
        
        // Restituisco un messaggio di errore
        return -1;
    }
    
    // L'operazione è andata a buon fine
    UNLOCK(&clientFilesMutex);

    print_client_files();

    return 1;
}


// Elimino la entry sulla tabella dei client che hanno fatto la open sui file
void remove_client_files(int fileDescriptor) {

    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // cancello il valore associato ai nodi
    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;
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
}

int contains_client_files(Node *fdList, char *path) {
	for (; fdList != NULL; fdList = fdList->next) {
		FileOpened *fileOpened = (FileOpened *) fdList->value;        
        if (strncmp(fileOpened->path, path, STRING_SIZE) == 0) {
			return 1;
		}
	}
	return 0;
}

void print_client_files() {
    icl_entry_t *bucket, *curr;

    printf("MAPPA:\n");
    
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
}

void destroy_storage() {

    icl_entry_t *bucket, *curr;

    // Itero tutta la mappa e cancello il fileLock
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

    // Itero tutta la mappa e cancello i nodi
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    Node *test = curr->data;
                    curr = curr->next;
                    free(test);
                }
            }
            curr = curr->next;
        }
    }
  
    // Cancello tutte le chiavi della mappa
    icl_hash_destroy(clientFiles, free, NULL);
    destroy_cache();
}
