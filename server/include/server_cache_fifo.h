#ifndef SERVER_CACHE_FIFO_H
#define SERVER_CACHE_FIFO_H

#include "../include/list_utils.h"

// struct NodeFIFO {
//     char *filePath;
//     struct NodeFIFO *next;
// };
// typedef struct NodeFIFO NodeFIFO;
// typedef NodeFIFO *CacheFIFO;

typedef struct Node NodeFIFO;
typedef NodeFIFO *CacheFIFO;

void insert_fifo(Node **cache, char *filePath);

void insert_update_fifo(Node **cache, char *filePath);

char *pop_fifo(Node **cache);

int contains_fifo(Node *cache, char *filePath);

void destroy_fifo(Node **cache);

void print_fifo(Node *cache);

#endif