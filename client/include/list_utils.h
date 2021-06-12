#ifndef LIST_UTIL_H
#define LIST_UTIL_H

#define STRING_SIZE 1024

struct Node {
    void *value;
    struct Node *next;
};
typedef struct Node Node;
typedef Node *List;

void create_list(Node **list, int (*fun_compare) (void*, void*));
void add_head(Node **list, void *value);
void add_tail(Node **list, void *value);
void *remove_head(Node **list);
void remove_value(Node **list, void *value);
int contains(Node *list, void *value);
void *get_value(Node *list, void *value);

int size(Node *list);
void clear_list(Node **list);

#endif

