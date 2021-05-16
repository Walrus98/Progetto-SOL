#ifndef LIST_UTIL_H
#define LIST_UTIL_H

struct Node {
    void *value;
    struct Node *next;
};
typedef struct Node Node;
typedef Node *List;

void add_tail(Node **list, void *value);
void add_head(Node **list, void *value);
void *remove_head(Node **list);
void remove_value(Node **list, void *value, void (*fun_compare) (void *value1, void *value2, void *size));
int contains(Node *list, void *value, void (*fun_compare) (void *value1, void *value2, void *size));

void clear_list(Node **list);
void print_list(Node *list);

// void insert_lru(NodeLRU **cache, char *filePath);

// void insert_update_lru(NodeLRU **cache, char *filePath);

// char *pop_lru(NodeLRU **cache);

// int get_lru(NodeLRU *cache, char *filePath);

// void destroy_lru(NodeLRU **cache);

// void print_lru(NodeLRU *cache);


#endif

