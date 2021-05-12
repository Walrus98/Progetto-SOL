#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../include/server_cache.h"

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

void pop_cache(Node **cache) {
    if (*cache != NULL) {
        Node *tempNode = *cache;
        *cache = (*cache)->next;
        free(tempNode);
    }
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