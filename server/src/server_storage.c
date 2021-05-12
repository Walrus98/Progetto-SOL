#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_storage.h"
#include "../include/server_cache.h"
#include "../include/icl_hash.h"

// Struttura dati che descrive il File in memoria
typedef struct File {
    char *filePath;
    char *fileContent;
    size_t fileSize;
    unsigned int modified;
} File;

// Capacità massima del server storage
static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

static icl_hash_t *storage;
static Cache cache;

void fifo_policy();

void create_storage(size_t fileCapacity, size_t storageCapacity) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    storage = icl_hash_create(fileCapacity, NULL, NULL);
}

void insert_storage(char *filePath, char *fileContent) {

    // Calcolo la dimensione del file
    size_t fileSize = strlen(filePath) + strlen(fileContent) + sizeof(size_t) + sizeof(unsigned int);

    // Se il file da inserire supera la capacità massima dello storage, annullo l'inserimento
    if (fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: il file non è stato inserito in quanto supera la capacità massima dello Storage.\n");
        return;
    }
    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta il massimo numero di File nello Storage. Applico politica FIFO\n");
        // exit(EXIT_FAILURE);
        fifo_policy();
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta la dimensione massima dello Storage. Applico politica FIFO\n");

        fifo_policy();
    }

    // Alloco la memoria necessaria per la creazione di un File
    File *file;
    if ((file = (File *) malloc(sizeof(File))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del file");
        exit(errno);
    }
    // Assegno al file creato il percorso, il contenuto passato e la dimensione
    file->filePath = filePath;
    file->fileContent = fileContent;
    file->fileSize = fileSize;

    // Se il file non è presente nella cache
    if (get_node(cache, file->filePath) == NULL) {
        // Inersico il file nella tabella hash
        icl_hash_insert(storage, file->filePath, file);
        // Inserisco il filePath nella cache
        insert_cache(&cache, file->filePath);

        // Aumento il contatore del numero dei file
        CURRENT_FILE_AMOUNT++;
        // Aumento la dimensione della struttura dati
        CURRENT_STORAGE_SIZE += file->fileSize;
    // Se il file è gia presente nella cache
    } else {
        // Prendo il riferimento al puntatore del File che è memorizzato nello storage
        File *oldFile = (File *) icl_hash_find(storage, file->filePath); 

        // Rimuovo il vecchio File dallo storage
        icl_hash_delete(storage, oldFile->filePath, NULL, NULL);
        // Inserisco il nuovo File con i valori aggiornati nello storage
        icl_hash_insert(storage, file->filePath, file);

        // Aggiorno la memoria attualmente occupata 
        CURRENT_STORAGE_SIZE -= oldFile->fileSize;
        CURRENT_STORAGE_SIZE += file->fileSize;

        File *newFile = (File *) icl_hash_find(storage, file->filePath);        
        printf("PATH %s \n", newFile->filePath);
        printf("CONTENT %s \n", newFile->fileContent);
    }

    // free(file);
}


void free_key(void *key) {
    // free((char *) key);
}

void free_data(void *data) {
    free((File *) data);
}

void destroy_storage() {
    printf("SUCCHIAMI IL CAZZO %d\n", icl_hash_destroy(storage, free_key, free_key));
    destroy_cache(&cache);
}

void print_storage() {

    icl_hash_dump(stdout, storage);
    print_cache(cache);

    printf("CURRENT_FILE_AMOUNT %ld\n", CURRENT_FILE_AMOUNT);
    printf("CURRENT_STORAGE_SIZE %ld\n", CURRENT_STORAGE_SIZE);

    printf("\n");
}

// POLICY DI RIMPIAZZAMENTO

void fifo_policy() {
    char *filePath = pop_cache(&cache);

    if (filePath != NULL) {
        icl_hash_delete(storage, filePath, NULL, NULL);
        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", filePath);
    } 
}