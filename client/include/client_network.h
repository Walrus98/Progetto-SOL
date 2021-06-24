#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#define OPEN_FILE 0
#define READ_FILE 1
#define READ_N_FILES 2
#define WRITE_FILE 3
#define APPEND_TO_FILE 4
#define LOCK_FILE 5
#define UNLOCK_FILE 6
#define CLOSE_FILE 6
#define REMOVE_FILE 7

char* SOCKET_PATH;

int openConnection(const char* sockname, int msec, const struct timespec abstime);

int closeConnection(const char* sockname);

int openFile(const char* pathname, int flags);

int readFile(const char* pathname, void** buf, size_t* size);

int readNFiles(int N, const char* dirname);

int writeFile(const char* pathname, const char* dirname);

int writeFile(const char *pathname, const char *dirname);

int closeFile(const char* pathname);

int removeFile(const char* pathname);

void write_file_directory(const char *dirName, char *fileName, char *buffer);

#endif