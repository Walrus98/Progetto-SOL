#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/list_utils.h"

static int (*compare) (void*, void*);

void create_list(Node **list, int (*fun_compare) (void*, void*)) {
	*list = NULL;

	compare = fun_compare;
}

void add_head(Node **list, void *value) {
    Node *newNode;
	if ((newNode = (Node *) malloc(sizeof(Node))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nodo nella lista");
		exit(errno);
	}
	newNode->value = value;
	newNode->next = *list;
	*list = newNode;
}

void add_tail(Node **list, void *value) {

    Node *newNode;
	if ((newNode = (Node *) malloc(sizeof(Node))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nodo nella lista");
		exit(errno);
	}
	newNode->value = value;
	newNode->next = NULL;
	
	if (*list == NULL) {
		*list = newNode;
	} else {
		Node *currentNode = *list;
        for (; currentNode->next != NULL; currentNode = currentNode->next);;

		currentNode->next = newNode;
	}
}

void *remove_head(Node **list) {
	void *value = NULL;
    if (*list != NULL) {
        Node *tempNode = *list;
		value = tempNode->value;
        *list = (*list)->next;
        free(tempNode);
    }
	return value;
}

void remove_value(Node **cache, void *value) {
	if (*cache != NULL) {
		Node *currentCache = *cache;
		Node *precNode = NULL;

		while (currentCache != NULL) {
			if (compare(currentCache->value, value)) {
				if (precNode == NULL) {
					Node *tempNode = *cache;
					*cache = (*cache)->next;
					free(tempNode);
					return;
				} else {
					Node *tempNode = *cache;
					precNode->next = currentCache->next;
					free(tempNode);
					return;
				}
			}
			precNode = currentCache;
			currentCache = currentCache->next;
		}
	}
}

int contains(Node *list, void *value) {
	for (; list != NULL; list = list->next) {
		if (compare(list->value, value)) {
			return 1;
		}
	}
	return 0;
}

void *get_value(Node *list, void *value) {
	for (; list != NULL; list = list->next) {
		if (compare(list->value, value)) {
			return list->value;
		}
	}
	return NULL;
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
