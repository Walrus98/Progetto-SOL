#include <stdio.h>
#include <stdlib.h>

#include "../include/server_cache_handler.h"
#include "../include/server_cache_fifo.h"

static CacheFIFO *cacheFIFO; 
static int STORAGE_POLICY;

void inizialize_policy(int replacementPolicy) {
    STORAGE_POLICY = replacementPolicy;
}

void replacement_file_cache(icl_hash_t *storage) {

    switch (STORAGE_POLICY) {
        case 0:
            ;
            char *filePath = pop_fifo(cacheFIFO);
            icl_hash_delete(storage, filePath, NULL, free);
            fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", filePath);
            break;
        case 1:
            break;
            // LRU
        default:
            break;
    }
}

void inset_file_cache(File *file) {
    switch (STORAGE_POLICY) {
        case 0:
            insert_fifo(cacheFIFO, file->filePath);
            break;
        case 1:
            break;
        default:
            break;
    }
}

void insert_update_file_cache(File *file) {
    switch (STORAGE_POLICY) {
        case 0:
            insert_update_fifo(cacheFIFO, file->filePath);
            break;
        case 1:
            break;
        default:
            break;
    }
}

int get_file_cache(File *file) {
    switch (STORAGE_POLICY) {
        case 0:
            return get_fifo(*cacheFIFO, file->filePath);
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
            destroy_fifo(cacheFIFO);
        case 1:
            break;
        default:
            break;
    }
}

void print_cache() {
    switch (STORAGE_POLICY) {
        case 0:
            print_fifo(*cacheFIFO);
        case 1:
            break;
        default:
            break;
    }
}