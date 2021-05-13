#include <stdio.h>
#include <stdlib.h>

#include "../include/server_cache_handler.h"
#include "../include/server_cache_fifo.h"
#include "../include/server_cache_lru.h"

static int STORAGE_POLICY;

static CacheFIFO cacheFIFO; 
static CacheLRU cacheLRU; 

void inizialize_policy(int replacementPolicy) {
    STORAGE_POLICY = replacementPolicy;
}

char *replacement_file_cache() {

    switch (STORAGE_POLICY) {
        case 0:
            return pop_fifo(&cacheFIFO);
            break;
        case 1:
            break;
            // LRU
        default:
            break;
    }
}

void insert_file_cache(char *filePath) { // File file
    switch (STORAGE_POLICY) {
        case 0:
            insert_fifo(&cacheFIFO, filePath); //file->filePath
            break;
        case 1:
            break;
        default:
            break;
    }
}

void insert_update_file_cache(char *filePath) { // File file
    switch (STORAGE_POLICY) {
        case 0:
            insert_update_fifo(&cacheFIFO, filePath); //file->filePath
            break;
        case 1:
            break;
        default:
            break;
    }
}

int contains_file_cache(char *filePath) { // File file
    switch (STORAGE_POLICY) {
        case 0:
            return get_fifo(cacheFIFO, filePath); //file->filePath
        case 1:
            break;
        default:
            break;
    }
    return 0;
}

void destroy_cache() {
    switch (STORAGE_POLICY) {
        case 0:
            destroy_fifo(&cacheFIFO);
        case 1:
            break;
        default:
            break;
    }
}

void print_cache() {
    switch (STORAGE_POLICY) {
        case 0:
            print_fifo(cacheFIFO);
        case 1:
            break;
        default:
            break;
    }
}