#ifndef SERVER_STORAGE_H
#define SERVER_STORAGE_H

typedef struct File {
    char *filePath;
    char *fileContent;
    size_t *fileSize;
    int *fifo;
    pthread_mutex_t *fileLock;
} File;


void create_storage(size_t fileCapacity, size_t storageCapacity);

int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock);

void *read_file(int fileDescriptor, char *filePath, int *bufferSize);

char *read_n_file(int nFiles, int *bufferSize);

int write_file(int fileDescriptor, char *filePath, char *fileContent);

int close_file(int fileDescriptor, char *filePath);

int remove_file(int fileDescriptor, char *filePath);

void disconnect_client(int fileDescriptor);

void destroy_storage();

void print_storage();

void print_storage_debug();

#endif
