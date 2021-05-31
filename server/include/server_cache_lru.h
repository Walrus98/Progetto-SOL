#ifndef SERVER_CACHE_LRU_H
#define SERVER_CACHE_LRU_H

#include "../include/server_cache_handler.h"

void insert_lru(Node **cache, File value);

void remove_lru(Node **cache);

#endif