#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

// Variabili condivise
size_t STORAGE_FILE_CAPACITY;
size_t STORAGE_CAPACITY;
int REPLACEMENT_POLICY;
size_t THREAD_WORKERS_AMOUNT;
char *SOCKET_FILE_NAME;

// Legge e parsa il file config.txt
void get_file_config();

#endif

