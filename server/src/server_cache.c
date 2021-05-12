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

void destroy_cache(Node **cache) {
    while (*cache != NULL) pop_cache(cache);
}

Node *get_node(Node *cache, char *filePath) {
	for (; cache != NULL; cache = cache->next) {
		if (strncmp(filePath, cache->filePath, STRING_BUFFER_SIZE) == 0) {
			return cache;
		}
	}
	return NULL;
}


void print_cache(Node *cache) {
	
    printf("==== Cache List ====\n");

	for (; cache != NULL; cache=cache->next) {
		printf("%s -> ", cache->filePath);
	}
	printf("\n\n");
}