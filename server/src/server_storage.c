#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_storage.h"
#include "../include/server_cache_handler.h"
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

void create_storage(size_t fileCapacity, size_t storageCapacity, int storagePolicy) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    storage = icl_hash_create(fileCapacity, NULL, NULL);
    inizialize_policy(storagePolicy);
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
        fprintf(stderr, "ATTENZIONE: raggiunta il massimo numero di File nello Storage.\n");
        
        char *fileToRemove = replacement_file_cache();

        icl_hash_delete(storage, fileToRemove, NULL, free);
        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", filePath);
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    while (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta la dimensione massima dello Storage.\n");
        
        char *fileToRemove = replacement_file_cache();

        icl_hash_delete(storage, fileToRemove, NULL, free);
        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", filePath);
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
    if (contains_file_cache(file->filePath) == 0) {
        // Inersico il file nella tabella hash
        icl_hash_insert(storage, file->filePath, file);
        // Inserisco il file nella cache
        insert_file_cache(file->filePath);

        // Aumento il contatore del numero dei file
        CURRENT_FILE_AMOUNT++;
        // Aumento la dimensione della struttura dati
        CURRENT_STORAGE_SIZE += file->fileSize;

    // Se il file è gia presente nella cache
    } else {
        // Prendo il riferimento al puntatore del File che è memorizzato nello storage
        File *oldFile = (File *) icl_hash_find(storage, file->filePath); 

        // Aggiorno la memoria attualmente occupata 
        CURRENT_STORAGE_SIZE -= oldFile->fileSize;

        // Rimuovo il vecchio File dallo storage
        icl_hash_delete(storage, oldFile->filePath, NULL, free);
        // Inserisco il nuovo File con i valori aggiornati nello storage
        icl_hash_insert(storage, file->filePath, file);
        // Aggiorno la cache con il File modificato
        insert_update_file_cache(file->filePath);

        // Aggiorno la memoria attualmente occupata 
        CURRENT_STORAGE_SIZE += file->fileSize;
    }
}

void destroy_storage() {
    icl_hash_destroy(storage, NULL, free);
    destroy_cache();
}

void print_storage() {
    icl_hash_dump(stdout, storage);
    print_cache();

    printf("CURRENT_FILE_AMOUNT %ld\n", CURRENT_FILE_AMOUNT);
    printf("CURRENT_STORAGE_SIZE %ld\n", CURRENT_STORAGE_SIZE);

    printf("\n");
}