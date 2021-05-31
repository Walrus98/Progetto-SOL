#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_cache_lru.h"

typedef struct FileLRU {
    File file;
    unsigned int used;
} FileLRU;

void insert_lru(Node **cache, File file) {
    FileLRU fileLRU;
    fileLRU.file = file;
    fileLRU.used = 0;

    add_tail(cache, &fileLRU);
}

void remove_lru(Node **cache) {

}


