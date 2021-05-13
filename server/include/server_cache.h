#ifndef SERVER_CACHE_H
#define SERVER_CACHE_H

struct Node {
    char *filePath;
    struct Node *next;
};
typedef struct Node Node;
typedef Node *Cache;

void insert_cache(Node **cache, char *filePath);

char *pop_cache(Node **cache);

void insert_update_cache(Node **cache, char *filePath);

void destroy_cache(Node **cache);

Node *get_node(Node *cache, char *filePath);

void print_cache(Node *cache);

#endif