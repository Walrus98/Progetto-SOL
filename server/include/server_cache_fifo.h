#ifndef SERVER_CACHE_FIFO_H
#define SERVER_CACHE_FIFO_H

#include "../include/server_cache_handler.h"

void insert_fifo(Node **cache, File file);

File *pop_fifo(Node **cache);

File *get_file_fifo(Node *cache, char *filePath);

void print_fifo(Node *cache);

void destroy_fifo(Node **cache);

#endif