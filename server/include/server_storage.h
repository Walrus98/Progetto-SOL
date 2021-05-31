#ifndef SERVER_STORAGE_H
#define SERVER_STORAGE_H

void create_storage(size_t fileCapacity, size_t storageCapacity, int storagePolicy);

int openFile(int fileDescriptor, char *filePath, int flagCreate, int flagLock);

void destroy_storage();

void print_storage();

#endif