#ifndef _SERVER_STORAGE_H
#define _SERVER_STORAGE_H

struct File {
    char *fileName;
    char *fileContent;
    size_t fileSize;
};
typedef struct File File;

void create_file_storage(size_t fileCapacity, size_t storageCapacity);

void insert_file_storage(char *fileName, char *fileContent, size_t fileSize);


#endif