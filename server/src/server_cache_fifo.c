#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/list_utils.h"
#include "../include/server_cache_fifo.h"

#define STRING_BUFFER_SIZE 512

void insert_fifo(Node **cache, char *filePath) {
	add_tail(cache, (void *) filePath);
}
	// Node *newNode;
	// if ((newNode = (Node *) malloc(sizeof(Node))) == NULL) {
    //     perror("ERRORE: impossibile allocare la memoria richiesta per la creazione del nodo in Cache");
	// 	exit(errno);
	// }
	// newNode->filePath = (void *) filePath;
	// newNode->next = NULL;
	
	// if (*cache == NULL) {
	// 	*cache = newNode;
	// } else {
	// 	Node *currentNode = *cache;
    //     for (; currentNode->next != NULL; currentNode = currentNode->next);
	// 	currentNode->next = newNode;
	// }

char *pop_fifo(Node **cache) {
	(char *) remove_head(cache);
}
	// char *filePath;
    // if (*cache != NULL) {
    //     Node *tempNode = *cache;
	// 	filePath = tempNode->filePath;
    //     *cache = (*cache)->next;
    //     free(tempNode);
    // }
	// return filePath;

void remove_fifo(Node **cache, char *filePath) {
	if (*cache != NULL) {
		Node *currentCache = *cache;
		Node *precNode = NULL;

		while (currentCache != NULL) {
			if (strncmp((char *) currentCache->value, filePath, STRING_BUFFER_SIZE) == 0) {
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

void insert_update_fifo(Node **cache, char *filePath) {
	remove_fifo(cache, filePath);
	insert_fifo(cache, filePath);
}

int contains_fifo(Node *cache, char *filePath) {
	for (; cache != NULL; cache = cache->next) {
		if (strncmp(filePath, (char *) cache->value, STRING_BUFFER_SIZE) == 0) {
			return 1;
		}
	}
	return 0;
}

void destroy_fifo(Node **cache) {
    // while (*cache != NULL) pop_fifo(cache);
	clear_list(cache);
}

void print_fifo(Node *cache) {
	
    printf("==== Cache FIFO List ====\n");

	print_list(cache);

	// for (; cache != NULL; cache=cache->next) {
	// 	printf("%s -> ", cache->filePath);
	// }
	// printf("\n\n");
}