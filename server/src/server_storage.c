#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_storage.h"
#include "../include/icl_hash.h"

static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

static icl_hash_t *hashTable;

void create_file_storage(size_t fileCapacity, size_t storageCapacity) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    hashTable = icl_hash_create(fileCapacity, NULL, NULL);
}

void insert_file_storage(char *fileName, char *fileContent, size_t fileSize) {

    File *file;
    if ((file = (File *) malloc(sizeof(File))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del file");
        exit(errno);
    }
    char *fileTempName;
    if ((fileName = (char *) malloc(sizeof(char *))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per il nome file");
        exit(errno);
    }
    char *fileTempContent;
    if ((fileContent = (char *) malloc(sizeof(char *))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la descrizione del file");
        exit(errno);
    }

    if (CURRENT_FILE_AMOUNT > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ERRORE: raggiunto il numero di File massimi");
        exit(EXIT_FAILURE);
    }

    if (CURRENT_STORAGE_SIZE + sizeof(file) > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: raggiunta la dimensione massima del File Storage");
        exit(EXIT_FAILURE);
    }

    strncmp(file->fileName, fileTempName, 256);
    strncmp(file->fileContent, fileTempContent, 2048);
    file->fileSize = fileSize;

    icl_hash_insert(hashTable, file->fileName, &file);

    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += sizeof(file);
}


// static size_t STORAGE_FILE_CAPACITY;
// static size_t STORAGE_CAPACITY;

// struct FileEntry {
// 	int info;
// 	struct FileEntry *next;
// };
// typedef struct FileEntry FileEntry;

// static FileEntry **fileStorage;

// void create_file_storage(size_t fileCapacity, size_t storageCapacity) {

//     if ((fileStorage = (FileEntry **) malloc(storageCapacity)) == NULL) {
//         perror("ERRORE: impossibile allocare la memoria richiesta per il server_storage");
//         exit(errno);
//     }
//     for (int i = 0; i < fileCapacity; i++) {
//         fileStorage[i] = NULL;
//     }

//     STORAGE_FILE_CAPACITY = fileCapacity;
//     STORAGE_CAPACITY = storageCapacity;
// }

// int toHash(int x) {
//     return x % 10;
// }

// void push_file_storage(int value) {

//     if (CURRENT_FILE_AMOUNT > STORAGE_FILE_CAPACITY) {
//         fprintf(stderr, "ERRORE: raggiunto il numero di File massimi");
//         exit(EXIT_FAILURE);
//     }

//     if (CURRENT_STORAGE_SIZE + sizeof(FileEntry) > STORAGE_CAPACITY) {
//         fprintf(stderr, "ERRORE: raggiunta la dimensione massima del File Storage");
//         exit(EXIT_FAILURE);
//     }

//     // Applico la funzione di hashing
//     int hashedValue = toHash(value);

//     // Creo l'elemento e lo inserisco nella tabella hash
//     FileEntry *nuovoElemento;    
//     if ((nuovoElemento = (FileEntry *) malloc(sizeof(FileEntry))) == NULL) {
//         perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nuovo File");
//         exit(errno);
//     }
//     nuovoElemento->next = fileStorage[hashedValue];
//     nuovoElemento->info = value;
//     fileStorage[hashedValue] = nuovoElemento;

//     CURRENT_FILE_AMOUNT++;
//     CURRENT_STORAGE_SIZE += sizeof(FileEntry);
// }

// void pop_file_storage(int value) {

// }

// void print_file_entries(FileEntry *fileEntries) {
//     for (; fileEntries != NULL; fileEntries = fileEntries->next) {
//         printf("[%d] -> ", fileEntries->info);
//     }
//     printf("[ ]");
// }

// void print_file_storage() {
//     for (int i = 0; i < STORAGE_FILE_CAPACITY; i++) {
//         print_file_entries(fileStorage[i]);
//         printf("\n");
//     }    

//     printf("Numero di File inseriti: %ld\n", CURRENT_FILE_AMOUNT);
//     printf("Dimensione del FileStorage: %ld byte\n", CURRENT_STORAGE_SIZE);
// }

// void aggiungi_testa(ElementoLista **lista, int valore) {
// 	ElementoLista *nuovoElemento = malloc(sizeof(ElementoLista));
// 	nuovoElemento->info = valore;
// 	nuovoElemento->next = *lista;
// 	*lista = nuovoElemento;
// }
// void aggiungi_coda(ElementoLista **lista, int valore) {
// 	ElementoLista *nuovoElemento = malloc(sizeof(ElementoLista));
// 	nuovoElemento->info = valore;
// 	nuovoElemento->next = NULL;
	
// 	if (*lista == NULL) {
// 		*lista = nuovoElemento;
// 	} else {
// 		ElementoLista *currentElemento = *lista;
//         for(; currentElemento->next != NULL; currentElemento = currentElemento->next);
// 		currentElemento->next = nuovoElemento;
// 	}
// }



