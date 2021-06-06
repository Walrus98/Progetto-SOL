#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_cache_fifo.h"

void insert_fifo(Node **cache, File file) {

	File *newFile = (File *) malloc(sizeof(File));

	char *filePath = malloc(sizeof(char) * (strlen(file.filePath) + 1));
	strcpy(filePath, file.filePath);
	newFile->filePath = filePath;

	char *fileContent = malloc(sizeof(char) * (strlen(file.fileContent) + 1));
	strcpy(fileContent, file.fileContent);
	newFile->fileContent = fileContent;
		
	size_t *size = (size_t *) malloc(sizeof(size_t));
	*size = *(file.fileSize);
	newFile->fileSize = size;

	int *locked = (int *) malloc(sizeof(int));
	*locked = *(file.fileLocked);
	newFile->fileLocked = locked;

	int *opens = (int *) malloc(sizeof(int));
	*opens = *(file.fileOpens);
	newFile->fileOpens = opens;

	add_tail(cache, newFile);
}

File *pop_fifo(Node **cache) {
	return (File *) remove_head(cache);
}

File *get_file_fifo(Node *cache, char *filePath) {
	for (; cache != NULL; cache = cache->next) {
		File *file = (File *) cache->value;

		if (strcmp(filePath, file->filePath) == 0) {
			return file;
		}
	}
	return NULL;
}

void print_fifo(Node *cache) {
	
    printf("==== Cache FIFO List ====\n");

	for (; cache != NULL; cache = cache->next) {
		File *file = ((File *) cache->value);
		printf("File Path %s\n", file->filePath);
		printf("File Content %s\n", file->fileContent);
		printf("File Size %ld\n", *(file->fileSize));
		printf("File Lock %d\n", *(file->fileLocked));
		printf("File Opens %d\n", *(file->fileOpens));
		
		printf("\n");
	}
}

void destroy_fifo(Node **cache) {
	
	for (Node *temp = *cache; temp != NULL; temp = temp->next) {
		File *file = (File *) temp->value;
		free(file->filePath);
		free(file->fileContent);
		free(file->fileSize);
		free(file->fileLocked);
		free(file->fileOpens);
		free(file);
	}

	clear_list(cache);
}

void remove_fifo(Node **cache, char *filePath) {
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