#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_cache.h"

#define STRING_BUFFER_SIZE 512

void insert_cache(Node **cache, char *filePath) {
	Node *newNode;
	if ((newNode = (Node *) malloc(sizeof(Node))) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nodo in Cache");
		exit(errno);
	}
	newNode->filePath = filePath;
	newNode->next = NULL;
	
	if (*cache == NULL) {
		*cache = newNode;
	} else {
		Node *currentNode = *cache;
        for (; currentNode->next != NULL; currentNode = currentNode->next);
		currentNode->next = newNode;
	}
}

char *pop_cache(Node **cache) {
	char *filePath;
    if (*cache != NULL) {
        Node *tempNode = *cache;
		filePath = tempNode->filePath;
        *cache = (*cache)->next;
        free(tempNode);
    }
	return filePath;
}

void remove_node(Node **cache, char *filePath) {
	if (*cache != NULL) {
		Node *currentCache = *cache;
		Node *precNode = NULL;

		while (currentCache != NULL) {
			if (strncmp(currentCache->filePath, filePath, STRING_BUFFER_SIZE) == 0) {
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

		// while (currentCache != NULL) {
		// 	if (strncmp(currentCache->filePath, filePath, STRING_BUFFER_SIZE) == 0) {
		// 		if (precNode == NULL) {
		// 			*cache = (*cache)->next;
		// 			currentCache = *cache;
		// 		} else {
		// 			precNode->next = currentCache->next;
		// 			currentCache = currentCache->next;
		// 		}
		// 	} else {
		// 		precNode = currentCache;
		// 		currentCache = currentCache->next;
		// 	}
}

void insert_update_cache(Node **cache, char *filePath) {
	remove_node(cache, filePath);
	insert_cache(cache, filePath);
}

Node *get_node(Node *cache, char *filePath) {
	for (; cache != NULL; cache = cache->next) {
		if (strncmp(filePath, cache->filePath, STRING_BUFFER_SIZE) == 0) {
			return cache;
		}
	}
	return NULL;
}

void destroy_cache(Node **cache) {
    while (*cache != NULL) pop_cache(cache);
}



void print_cache(Node *cache) {
	
    printf("==== Cache List ====\n");

	for (; cache != NULL; cache=cache->next) {
		printf("%s -> ", cache->filePath);
	}
	printf("\n\n");
}