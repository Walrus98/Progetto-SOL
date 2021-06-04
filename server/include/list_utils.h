#ifndef LIST_UTIL_H
#define LIST_UTIL_H

#define STRING_SIZE 1024

struct Node {
    void *value;
    struct Node *next;
};
typedef struct Node Node;
typedef Node *List;

void add_tail(Node **list, void *value);
void add_head(Node **list, void *value);
void *remove_head(Node **list);
void remove_value(Node **cache, char *filePath);
int contains(Node *list, void *value);

int size(Node *list);
void clear_list(Node **list);

#endif

