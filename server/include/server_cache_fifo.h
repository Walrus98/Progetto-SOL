#ifndef SERVER_CACHE_FIFO_H
#define SERVER_CACHE_FIFO_H

struct Node {
    char *filePath;
    struct Node *next;
};
typedef struct Node Node;
typedef Node *CacheFIFO;

void insert_fifo(Node **cache, char *filePath);

void insert_update_fifo(Node **cache, char *filePath);

char *pop_fifo(Node **cache);

int get_fifo(Node *cache, char *filePath);

void destroy_fifo(Node **cache);

void print_fifo(Node *cache);

#endif