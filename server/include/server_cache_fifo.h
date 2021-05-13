#ifndef SERVER_CACHE_FIFO_H
#define SERVER_CACHE_FIFO_H

struct NodeFIFO {
    char *filePath;
    struct NodeFIFO *next;
};
typedef struct NodeFIFO NodeFIFO;
typedef NodeFIFO *CacheFIFO;

void insert_fifo(NodeFIFO **cache, char *filePath);

void insert_update_fifo(NodeFIFO **cache, char *filePath);

char *pop_fifo(NodeFIFO **cache);

int get_fifo(NodeFIFO *cache, char *filePath);

void destroy_fifo(NodeFIFO **cache);

void print_fifo(NodeFIFO *cache);

#endif