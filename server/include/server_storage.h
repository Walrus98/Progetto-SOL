#ifndef SERVER_STORAGE_H
#define SERVER_STORAGE_H

void create_storage(size_t fileCapacity, size_t storageCapacity, int storagePolicy);

int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock);

void *read_file(int fileDescriptor, char *filePath, int *bufferSize);

void write_file(int fileDescriptor, char *filePath, char *fileContent);

void remove_client_files();

void destroy_storage();

void print_storage();

#endif