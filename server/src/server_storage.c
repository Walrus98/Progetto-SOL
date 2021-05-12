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

void create_storage(size_t fileCapacity, size_t storageCapacity) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    storage = icl_hash_create(fileCapacity, NULL, NULL);
}

void insert_storage(char *filePath, char *fileContent) {

    // Calcolo la dimensione del file
    size_t fileSize = strlen(filePath) + strlen(fileContent) + sizeof(size_t) + sizeof(unsigned int);

    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    if (CURRENT_FILE_AMOUNT > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ERRORE: raggiunto il numero di File massimi");
        exit(EXIT_FAILURE);
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    if (CURRENT_STORAGE_SIZE + fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: raggiunta la dimensione massima del File Storage");
        exit(EXIT_FAILURE);
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

    // Inersico il file nella tabella hash
    icl_hash_insert(storage, file->filePath, &file);
    // Inserisco il filePath nella cache
    insert_cache(&cache, file->filePath);

    // Aumento il contatore del numero dei file
    CURRENT_FILE_AMOUNT++;
    // Aumento la dimensione della struttura dati
    CURRENT_STORAGE_SIZE += file->fileSize;

    free(file);
}

void destroy_storage() {
    icl_hash_destroy(storage, NULL, NULL);
    destroy_cache(&cache);
}

void print_storage() {
    icl_hash_dump(stdout, storage);
    print_cache(cache);

    printf("CURRENT_FILE_AMOUNT %ld\n", CURRENT_FILE_AMOUNT);
    printf("CURRENT_STORAGE_SIZE %ld\n", CURRENT_STORAGE_SIZE);

    printf("\n");
}