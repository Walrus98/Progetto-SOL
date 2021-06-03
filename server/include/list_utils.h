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
int contains(Node *list, void *value);

int size(Node *list);
void clear_list(Node **list);

#endif

