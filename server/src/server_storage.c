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

void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy);
void insert_storage(File file);
void print_storage();
void destroy_storage();

int openFile(int fileDescriptor, char *filePath, int flagCreate, int flagLock);

void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    inizialize_policy(replacementPolicy);

    clientFiles = icl_hash_create(100, NULL, NULL);
}

// - int openFile(const char* pathname, int flags)
// Richiesta di apertura o di creazione di un file. La semantica della openFile dipende dai flags passati come secondo
// argomento che possono essere O_CREATE ed O_LOCK. Se viene passato il flag O_CREATE ed il file esiste già
// memorizzato nel server, oppure il file non esiste ed il flag O_CREATE non è stato specificato, viene ritornato un
// errore. In caso di successo, il file viene sempre aperto in lettura e scrittura, ed in particolare le scritture possono
// avvenire solo in append. Se viene passato il flag O_LOCK (eventualmente in OR con O_CREATE) il file viene
// aperto e/o creato in modalità locked, che vuol dire che l’unico che può leggere o scrivere il file ‘pathname’ è il
// processo che lo ha aperto. Il flag O_LOCK può essere esplicitamente resettato utilizzando la chiamata unlockFile,
// descritta di seguito.
// Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene settato opportunamente.

int openFile(int fileDescriptor, char *filePath, int flagCreate, int flagLock) {

    File *file = get_file_cache(filePath);

    if ((flagCreate == 1 && file != NULL) || (flagCreate == 0 && file == NULL)) {

        printf("File Descriptor: %d\n", fileDescriptor);
        print_cache();
        return 0;
    }

    if (flagCreate == 1 && file == NULL) {

        File newFile;
        newFile.filePath = filePath;
        newFile.fileContent = "";
        newFile.fileSize = get_file_size(newFile);  

        // newFile.fdList = NULL;

        LOCK(&clientFilesMutex);

        int *fd = (int *) malloc(sizeof(int));
        *fd = fileDescriptor;

        Node *fdList = (Node *) icl_hash_find(clientFiles, fd);

        if (fdList == NULL) {
            Node *newFdList = NULL;            
            
            char *filePath = malloc(sizeof(char) * (strlen(newFile.filePath) + 1));
            strcpy(filePath, newFile.filePath);

            add_head(&newFdList, filePath);

            icl_hash_insert(clientFiles, fd, newFdList);
        } else {

            char *filePath = malloc(sizeof(char) * (strlen(newFile.filePath) + 1));
            strcpy(filePath, newFile.filePath);
            
            add_head(&fdList, filePath);
            
            // prova
            // icl_hash_delete(clientFiles, fd, free, free);
            // icl_hash_insert(clientFiles, fd, fdList);
        }


        UNLOCK(&clientFilesMutex);

        // int *fd = (int *) malloc(sizeof(int));
        // *fd = fileDescriptor;
        // add_head(&(newFile.fdList), fd);
        

        insert_storage(newFile);
        
        return 1;
    }

    if (flagCreate == 0 && file != NULL) {

        // add_head(&(file->fdList), &fileDescriptor);

        
        file->fileSize = get_file_size(*file);
        
        LOCK(&clientFilesMutex);
        
        int *fd = (int *) malloc(sizeof(int));
        *fd = fileDescriptor;

        Node *fdList = (Node *) icl_hash_find(clientFiles, fd);

        if (fdList == NULL) {
            Node *newFdList = NULL;            
            
            char *filePath = malloc(sizeof(char) * (strlen(file->filePath) + 1));
            strcpy(filePath, file->filePath);

            add_head(&newFdList, filePath);

            icl_hash_insert(clientFiles, fd, newFdList);
        } else {

            char *filePath = malloc(sizeof(char) * (strlen(file->filePath) + 1));
            strcpy(filePath, file->filePath);
            
            add_head(&fdList, filePath);
            
            // prova
            // icl_hash_delete(clientFiles, fd, NULL, free);
            // icl_hash_insert(clientFiles, fd, fdList);
        }


        UNLOCK(&clientFilesMutex);

        return 1;
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

void destroy_storage() {

    icl_entry_t *bucket, *curr;

    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    free(temp->value);
                }
            }
            curr = curr->next;
        }
    }

    icl_hash_destroy(clientFiles, free, free);
    destroy_cache();
}

void removeKey(int fileDescriptor) {

    printf("YOLO!\n");

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        free(curr->value);
    }

    icl_hash_delete(clientFiles, &fileDescriptor, free, free);

}


// int
// icl_hash_dump(FILE* stream, icl_hash_t* ht)
// {
//     icl_entry_t *bucket, *curr;
//     int i;

//     if(!ht) return -1;

//     printf("==== Storage Table ====\n");

//     for(i=0; i<ht->nbuckets; i++) {
//         bucket = ht->buckets[i];
//         for(curr=bucket; curr!=NULL; ) {
//             if(curr->key)
//                 fprintf(stream, "%p -> %p\n", curr->key, curr->data);
//             curr=curr->next;
//         }
//     } 
//     printf("\n");

//     return 0;
// }
