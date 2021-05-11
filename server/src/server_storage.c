#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_storage.h"
#include "../include/icl_hash.h"
// #include "../include/utils_list.h"

struct File {
    char *filePath;
    char *fileContent;
    size_t fileSize;
    unsigned int modified;
};
typedef struct File File;

struct Node {
    char *filePath;
    struct Node *next;
};
typedef struct Node Node;

static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

static icl_hash_t *storage;
static Node *cache;

// =============== LISTA ===============

void pop_cache(Node **list) {
    if (*list != NULL) {
        Node *tempNode = *list;
        *list = (*list)->next;
        free(tempNode);
    }
}

void destroy_cache(Node **list) {
    while (*list != NULL) pop_cache(list);
}

Node *insert_cache(char *filePath) {
    Node *newElement = (Node *) malloc(sizeof(Node));
    newElement->filePath = filePath;
    newElement->next = NULL;

    Node *currentList = cache;
    while (currentList->next != NULL) {
        Node *temp = currentList;
        currentList = currentList->next;
        free(temp);
    }
    currentList->next = newElement;


    // destroy_cache(&currentList);


    return newElement;    
}



// =============== TABELLA HASH ===============

void create_storage(size_t fileCapacity, size_t storageCapacity) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    storage = icl_hash_create(fileCapacity, NULL, NULL);
    // add_tail(&list, Node);
}

void insert_storage(char *fileName, char *fileContent) {

    File *file;
    if ((file = (File *) malloc(sizeof(File))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del file");
        exit(errno);
    }

    file->filePath = fileName;
    file->fileContent = fileContent;
    file->fileSize = strlen(fileName) + strlen(fileContent) + sizeof(size_t) + sizeof(unsigned int);

    if (CURRENT_FILE_AMOUNT > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ERRORE: raggiunto il numero di File massimi");
        exit(EXIT_FAILURE);
    }

    if (CURRENT_STORAGE_SIZE + file->fileSize > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: raggiunta la dimensione massima del File Storage");
        exit(EXIT_FAILURE);
    }

    icl_hash_insert(storage, file->filePath, &file);
    insert_cache(file->filePath);

    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += file->fileSize;

    free(file);
}

void destroy_storage() {
    icl_hash_destroy(storage, NULL, NULL);
    destroy_cache(&cache);
}

void print_storage() {
    icl_hash_dump(stdout, storage);

    printf("CURRENT_FILE_AMOUNT %ld\n", CURRENT_FILE_AMOUNT);
    printf("CURRENT_STORAGE_SIZE %ld\n", CURRENT_STORAGE_SIZE);
}