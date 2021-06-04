#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/list_utils.h"
#include "../include/server_cache_handler.h"

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

void remove_value(Node **cache, char *filePath) {
	if (*cache != NULL) {
		Node *currentCache = *cache;
		Node *precNode = NULL;

		while (currentCache != NULL) {
			if (strncmp((char *) currentCache->value, filePath, STRING_SIZE) == 0) {
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
		if (strncmp((char *) list->value, (char *) value, STRING_SIZE) == 0) {
			return 1;
		}
	}
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
