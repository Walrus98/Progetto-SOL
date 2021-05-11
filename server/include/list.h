#ifndef LIST_H
#define LIST_H

struct Node {
    void *el;
    struct Node *next;
};
typedef struct Node Node;

void add_tail();

void print_list();

#endif