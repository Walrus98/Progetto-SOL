#ifndef SERVER_CACHE_H
#define SERVER_CACHE_H

struct Node {
    char *filePath;
    struct Node *next;
};
typedef struct Node Node;
typedef Node *Cache;

void insert_cache(Node **cache, char *filePath);

void destroy_cache(Node **cache);

void pop_cache(Node **cache);

void print_cache(Node *cache);

#endif