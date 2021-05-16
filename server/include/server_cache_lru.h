#ifndef SERVER_CACHE_LRU_H
#define SERVER_CACHE_LRU_H

#include "../include/list_utils.h"

typedef struct LRU {
    char *filePath;
    int crontab;
} LRU;

typedef struct Node NodeLRU;
typedef NodeLRU *CacheLRU;

void insert_lru(Node **cache, LRU value);

void insert_update_lru(Node **cache, LRU value);

char *pop_lru(Node **cache);

int contains_lru(Node *cache, LRU value);

void destroy_lru(Node **cache);

void print_lru(Node *cache);

#endif