#ifndef SERVER_CACHE_HANDLER_H
#define SERVER_CACHE_HANDLER_H

#define FIFO_POLICY 0
#define LRU_POLICY 1

#include "../include/list_utils.h"

typedef struct File {
    char *filePath;
    char *fileContent;
    // Node *fdList;
    size_t fileSize;
} File;

void inizialize_policy(int replacementPolicy);

size_t get_file_size(File file);

void insert_file_cache(File file);

File *replacement_file_cache();

File *get_file_cache(char *filePath);

void print_cache();

void destroy_cache();

// char *replacement_file_cache();
// void insert_update_file_cache(char *file);

#endif