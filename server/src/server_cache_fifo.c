#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_cache_fifo.h"

void insert_fifo(Node **cache, File file) {

	File *newFile = (File *) malloc(sizeof(File));
	newFile->filePath = file.filePath;
	newFile->fileContent = file.fileContent;



	newFile->fdList = file.fdList;
	newFile->fileSize = file.fileSize;

	add_tail(cache, newFile);
}

File *pop_fifo(Node **cache) {
	return (File *) remove_head(cache);
}

File *get_file_fifo(Node *cache, char *filePath) {
	for (; cache != NULL; cache = cache->next) {
		File *tempFile = (File *) cache->value;

		
		if (tempFile != NULL) {
			// printf("File Path %s\n", filePath);
			printf("File Path La Vendetta %s\n", tempFile->filePath);
			printf("File Content %s\n", tempFile->fileContent);
			printf("File Size %ld\n", tempFile->fileSize);
			printf("\n");
		}

		// if (strcmp(filePath, tempFile->filePath) == 0) {
		// 	return (File *) cache->value;
		// }
	}
	return NULL;
}

void print_fifo(Node *cache) {
	
    printf("==== Cache FIFO List ====\n");

	for (; cache != NULL; cache = cache->next) {
		File *file = ((File *) cache->value);
		printf("File Path %s\n", file->filePath);
		printf("File Content %s\n", file->fileContent);
		printf("File Size %ld\n", file->fileSize);
		printf("\n");
	}
}

void destroy_fifo(Node **cache) {
	
	for (Node *temp = *cache; temp != NULL; temp = temp->next) {
		File *file = ((File *) temp->value);
		clear_list(&(file->fdList));
		free(file);
	}

	clear_list(cache);
}

// void remove_fifo(Node **cache, char *filePath) {
// 	if (*cache != NULL) {
// 		Node *currentCache = *cache;
// 		Node *precNode = NULL;

// 		while (currentCache != NULL) {
// 			if (strncmp((char *) currentCache->value, filePath, STRING_BUFFER_SIZE) == 0) {
// 				if (precNode == NULL) {
// 					Node *tempNode = *cache;
// 					*cache = (*cache)->next;
// 					free(tempNode);
// 					return;
// 				} else {
// 					Node *tempNode = *cache;
// 					precNode->next = currentCache->next;
// 					free(tempNode);
// 					return;
// 				}
// 			}
// 			precNode = currentCache;
// 			currentCache = currentCache->next;
// 		}
// 	}
// }

// void insert_update_fifo(Node **cache, char *filePath) {
// 	remove_fifo(cache, filePath);
// 	insert_fifo(cache, filePath);
// }

// int contains_fifo(Node *cache, char *filePath) {
// 	for (; cache != NULL; cache = cache->next) {
// 		if (strncmp(filePath, (char *) cache->value, STRING_BUFFER_SIZE) == 0) {
// 			return 1;
// 		}
// 	}
// 	return 0;
// }

// void destroy_fifo(Node **cache) {
//     // while (*cache != NULL) pop_fifo(cache);
// 	clear_list(cache);
// }

// void print_fifo(Node *cache) {
	
//     printf("==== Cache FIFO List ====\n");

// 	print_list(cache);

// 	// for (; cache != NULL; cache=cache->next) {
// 	// 	printf("%s -> ", cache->filePath);
// 	// }
// 	// printf("\n\n");
// }