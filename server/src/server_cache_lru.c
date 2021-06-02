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

    FileLRU *newFile = (FileLRU *) malloc(sizeof(FileLRU));
    newFile->file.filePath = file.filePath;
	newFile->file.fileContent = file.fileContent;
	newFile->file.fileSize = file.fileSize;
    newFile->used = 0;

    add_tail(cache, newFile);
}

void remove_lru(Node **cache) {

}


