#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "../include/storage.h"
#include "../include/pthread_utils.h"
#include "../include/icl_hash.h"
#include "../../core/include/list_utils.h"

typedef struct File {
    char *filePath;
    char *fileContent;
    size_t *fileSize;
    int *fifo;
    pthread_mutex_t *fileLock;
} File;


// Capacità massima del server storage
static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

// Capacità corrente del server storage
static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

static icl_hash_t *storage;
static pthread_mutex_t STORAGE_LOCK = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t CAPACITY_LOCK = PTHREAD_MUTEX_INITIALIZER;

int hash_compare(void *a, void *b) {

    File *file = (File *) a;
    char *filePath = (char *) b;

    int res = strncmp(file->filePath, filePath, strlen(file->filePath) + 1);

    return res == 0 ? 1 : 0;
}

int list_compare(void *a, void *b) {
    return *((int *) a) == *((int *) b);
}

// Inizalizza la Mappa
void create_storage(size_t fileCapacity, size_t storageCapacity);
// Inserisce un file all'interno dello storage, se è già presente lo modifica
int insert_file_storage(int fileDescriptor, char *filePath, char *fileContent);
// Rimuove il file dettato dalla politica di rimpiazzamento
void replace_file_storage();
// Rimuove il file selezionato dalla mappa
int remove_file_storage(File *fileToRemove);
// Restituisco il file che ha come path quello passato per parametro
File *get_file(char *filePath);
// Stampo la mappa
void print_storage();
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
    storage = icl_hash_create(fileCapacity, NULL, hash_compare);
}

int insert_file_storage(int fileDescriptor, char *filePath, char *fileContent) {

    LOCK(&STORAGE_LOCK);

    // Controllo se il file è già presente o meno nello storage
    File *file = get_file(filePath);

    // Se il file è diverso da null, significa che è già presente e quindi devo modificarlo (append)
    if (file != NULL) {
        remove_file_storage(file);

        UNLOCK(&STORAGE_LOCK);
        insert_file_storage(fileDescriptor, filePath, fileContent);
    }

    size_t pathLength = strlen(filePath) + 1;
    size_t contentLength = strlen(fileContent) + 1;
    size_t fileSize = pathLength + contentLength;

    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    LOCK(&CAPACITY_LOCK);
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: raggiunta il massimo numero di File nello Storage.\n");
        replace_file_storage();
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    LOCK(&CAPACITY_LOCK);
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        UNLOCK(&CAPACITY_LOCK);
        fprintf(stderr, "ATTENZIONE: raggiunta la dimensione massima dello Storage.\n");
        replace_file_storage();
    }

    File *newFile = (File *) malloc(sizeof(File));
   
	char *path = (char *) malloc(pathLength);
	strncpy(path, filePath, pathLength);
	newFile->filePath = path;

    char *content = (char *) malloc(contentLength);
	strncpy(content, fileContent, contentLength);
	newFile->fileContent = content;

    size_t *size = (size_t *) malloc(sizeof(size_t));
	*size = fileSize;
	newFile->fileSize = size;

    int *fifo = (int *) malloc(sizeof(int));
    *fifo = CURRENT_FILE_AMOUNT;
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
    UNLOCK(&CAPACITY_LOCK);
    
    UNLOCK(&STORAGE_LOCK);

    return 0;
}

void replace_file_storage() {

    int min = 0;
    File *fileToRemove;
    
    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL && min < *(file->fifo)) {
                min = *(file->fifo);
                fileToRemove = file;
            }
            curr = curr->next;
        }
    }
    
    fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);        

    remove_file_storage(fileToRemove);
}


int remove_file_storage(File *fileToRemove) {

    LOCK(fileToRemove->fileLock);

    if (fileToRemove == NULL) {
        UNLOCK(fileToRemove->fileLock);
        return -1;
    }

    LOCK(&CAPACITY_LOCK);
    CURRENT_STORAGE_SIZE -= *(fileToRemove->fileSize);
    CURRENT_FILE_AMOUNT--;
    UNLOCK(&CAPACITY_LOCK);

    Node* usersList = icl_hash_find(storage, fileToRemove);
    for (Node *currentList = usersList; currentList != NULL; ) {
        free(currentList->value);
        Node *temp = currentList;
        currentList = currentList->next;
        free(temp);
    }

    free(fileToRemove->filePath);
    free(fileToRemove->fileContent);
    free(fileToRemove->fifo);
    free(fileToRemove->fileSize);
    free(fileToRemove);
    
    pthread_mutex_t *lock = fileToRemove->fileLock;
    UNLOCK(lock);

    free(lock);

    // icl_hash_delete(storage, fileToRemove, NULL, NULL);

    return 0;
}

File *get_file(char *filePath) {

    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL && hash_compare(file, filePath) == 1) {
                return file;
            }
            curr = curr->next;
        }
    }
    return NULL;
}

void print_storage() {

}

void destroy_storage() {

    LOCK(&STORAGE_LOCK);

    for (int i = 0; i < storage->nbuckets; i++) {
        icl_entry_t *bucket = storage->buckets[i];
        icl_entry_t *curr;
        for (curr = bucket; curr != NULL; ) {
            File *file = (File *) curr->key;
            if (file != NULL) {
                remove_file_storage(file);
            }
            curr = curr->next;
        }
    }

    icl_hash_destroy(storage, free, free);

    UNLOCK(&STORAGE_LOCK);
}


int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock) { 
    return 0;
}
void *read_file(int fileDescriptor, char *filePath, int *bufferSize) {
    return NULL;
}
char *read_n_file(int nFiles, int *bufferSize) {
    return NULL;
}
int write_file(int fileDescriptor, char *filePath, char *fileContent) {
    return 0;
}
int close_file(int fileDescriptor, char *filePath) {
    return 0;
}
int remove_file(int fileDescriptor, char *filePath) {
    return 0;
}
void disconnect_client(int fileDescriptor) {

}

