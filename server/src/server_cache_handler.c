#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/server_cache_handler.h"
#include "../include/server_cache_fifo.h"
#include "../include/server_cache_lru.h"

static int REPLACEMENT_POLICY;

static Node* cacheFIFO = NULL;
// static Node* cacheLRU;

void inizialize_policy(int replacementPolicy) {
    REPLACEMENT_POLICY = replacementPolicy;
}

size_t get_file_size(File file) {
    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            ;
            return strlen(file.filePath) + strlen(file.fileContent) + 2 + sizeof(size_t);
        default:
            break;    
    }
    return 0;

}

void insert_file_cache(File file) {
    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            insert_fifo(&cacheFIFO, file);
            break;
        case LRU_POLICY:
            // insert_lru(&cacheLRU, file);
            break;
        default:
            break;
    }
}

File *replacement_file_cache() {
    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            return pop_fifo(&cacheFIFO);
        default:
            break;
    }
    return NULL;
}

File *get_file_cache(char *filePath) {
    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            return get_file_fifo(cacheFIFO, filePath);   
    }
    return NULL;
}


void print_cache() {
    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            print_fifo(cacheFIFO);
        case 1:
            break;
        default:
            break;
    }
}

void destroy_cache() {

    switch (REPLACEMENT_POLICY) {
        case FIFO_POLICY:
            destroy_fifo(&cacheFIFO);
            break;
        case 1:
            break;
        default:
            break;
    }
}


// File *replacement_file_cache() {

//     switch (REPLACEMENT_POLICY) {
//         case FIFO_POLICY:
//             return pop_fifo(&cacheFIFO);
//         default:
//             break;
//     }

//     return NULL;
// }

// void insert_update_file_cache(char *filePath) { // File file
//     switch (REPLACEMENT_POLICY) {
//         case 0:
//             insert_update_fifo(&cacheFIFO, filePath); //file->filePath
//             break;
//         case 1:
//             break;
//         default:
//             break;
//     }
// }

// int contains_file_cache(char *filePath) { // File file
//     switch (REPLACEMENT_POLICY) {
//         case 0:
//             return contains_fifo(cacheFIFO, filePath); //file->filePath
//         case 1:
//             break;
//         default:
//             break;
//     }
//     return 0;
// }

