#ifndef _SERVER_STORAGE_H
#define _SERVER_STORAGE_H

void create_storage(size_t fileCapacity, size_t storageCapacity);

void insert_storage(char *fileName, char *fileContent);

void destroy_storage();

void print_storage();

#endif