#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../include/server_cache_handler.h"
#include "../include/pthread_utils.h"
#include "../include/server_cache_fifo.h"
#include "../include/server_cache_lru.h"

static int REPLACEMENT_POLICY;

static Node* cacheFIFO = NULL;
// static Node* cacheLRU;

static pthread_mutex_t cacheMutex = PTHREAD_MUTEX_INITIALIZER;

void inizialize_policy(int replacementPolicy) {
    REPLACEMENT_POLICY = replacementPolicy;
}

size_t get_file_size(File file) {
    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            ;
            int size = strlen(file.filePath) + strlen(file.fileContent) + 2 + sizeof(size_t) + sizeof(File) + (sizeof(int) * 2);
            UNLOCK(&cacheMutex);
            return size;
        default:
            break;    
    }

    UNLOCK(&cacheMutex);
    return 0;

}

void insert_file_cache(File file) {
    
    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            insert_fifo(&cacheFIFO, file);
            UNLOCK(&cacheMutex);
            break;
        case LRU_POLICY:
            // insert_lru(&cacheLRU, file);
            break;
        default:
            break;
    }

    UNLOCK(&cacheMutex);
}

void remove_file_cache(char *filePath) {    
    
    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            ;
            remove_fifo(&cacheFIFO, filePath);
            break;
        default:
            break;
    }
    
    UNLOCK(&cacheMutex);
}

File *replacement_file_cache() {
    
    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            ;
            File *fileToRemove = pop_fifo(&cacheFIFO);
            UNLOCK(&cacheMutex);
            return fileToRemove;
        default:
            break;
    }
    
    UNLOCK(&cacheMutex);

    return NULL;
}

File *get_file_cache(char *filePath) {
    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            ;
            File *file = get_file_fifo(cacheFIFO, filePath);  
            UNLOCK(&cacheMutex);
            return file; 
    }
    
    UNLOCK(&cacheMutex);

    return NULL;
}


int get_file_lock(File *file) {
    
    LOCK(&cacheMutex);

    int locked = *(file->fileLocked);
    
    UNLOCK(&cacheMutex);

    return locked;
}

void set_file_lock(File *file, int flagLock) {

    LOCK(&cacheMutex);

    *(file->fileLocked) = flagLock;

    UNLOCK(&cacheMutex);
}

void increase_file_opens(File *file) {

    LOCK(&cacheMutex);

    (*(file->fileOpens))++;

    UNLOCK(&cacheMutex);
}


void decrease_file_opens(File *file) {

    LOCK(&cacheMutex);

    (*(file->fileOpens))--;

    UNLOCK(&cacheMutex);
}

int get_files_opens(File *file) {

    LOCK(&cacheMutex);

    int opens = *(file->fileOpens);
    
    UNLOCK(&cacheMutex);

    return opens;
}

char* get_n_files_cache(int nFiles) {
    
    LOCK(&cacheMutex);

    char *buffer = NULL;

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            buffer = get_n_files_fifo(cacheFIFO, nFiles);
            UNLOCK(&cacheMutex);
            return buffer;
        default:
            break;
    }
    
    UNLOCK(&cacheMutex);

    return NULL;
}

void print_cache() {
    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            print_fifo(cacheFIFO);
            UNLOCK(&cacheMutex);
        default:
            break;
    }
    
    UNLOCK(&cacheMutex);
}

void destroy_cache() {

    LOCK(&cacheMutex);

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            destroy_fifo(&cacheFIFO);
            UNLOCK(&cacheMutex);
            break;
        default:
            break;
    }
    
    UNLOCK(&cacheMutex);
}