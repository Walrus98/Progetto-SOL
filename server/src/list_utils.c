#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../include/list_utils.h"

void add_tail(Node **list, void *value) {
    Node *newNode;
	if ((newNode = (Node *) malloc(sizeof(Node))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nodo in Cache");
		exit(errno);
	}
	newNode->value = value;
	newNode->next = NULL;
	
	if (*list == NULL) {
		*list = newNode;
	} else {
		Node *currentNode = *list;
        for (; currentNode->next != NULL; currentNode = currentNode->next);
		currentNode->next = newNode;
	}
}

void add_head(Node **list, void *value) {
    Node *newNode;
	if ((newNode = (Node *) malloc(sizeof(Node))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nodo in Cache");
		exit(errno);
	}
	newNode->value = value;
	newNode->next = *list;
	*list = newNode;
}

void *remove_head(Node **list) {
	void *value;
    if (*list != NULL) {
        Node *tempNode = *list;
		value = tempNode->value;
        *list = (*list)->next;
        free(tempNode);
    }
	return value;
}

void remove_value(Node **list, void *value, void (*fun_compare) (void *value1, void *value2, void *size)) {
}

int contains(Node *list, void *value,  void (*fun_compare) (void *value1, void *value2, void *size)) {
	return 0;
}

int size(Node *list) {
	int index = 0;
	for (; list != NULL; list = list->next) {
		index++;
	}
	return index;
}

void clear_list(Node **list) {
    while (*list != NULL) remove_head(list);
}

void print_list(Node *list) {

    for (; list != NULL; list = list->next) {
        printf("%s -> ", (char *) list->value);
    }
    printf("\n\n");
}
