#ifndef SERVER_CACHE_LRU_H
#define SERVER_CACHE_LRU_H

struct NodeLRU {
    char *filePath;
    int crontab;
    struct NodeLRU *next;
};
typedef struct NodeLRU NodeLRU;
typedef NodeLRU *CacheLRU;

void insert_lru(NodeLRU **cache, char *filePath);

void insert_update_lru(NodeLRU **cache, char *filePath);

char *pop_lru(NodeLRU **cache);

int get_lru(NodeLRU *cache, char *filePath);

void destroy_lru(NodeLRU **cache);

void print_lru(NodeLRU *cache);

#endif