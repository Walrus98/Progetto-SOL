#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "../include/server_storage.h"
#include "../include/server_cache_handler.h"
#include "../include/icl_hash.h"
#include "../include/pthread_utils.h"

// Capacità massima del server storage
static size_t STORAGE_FILE_CAPACITY;
static size_t STORAGE_CAPACITY;

// Capacità corrente del server storage
static size_t CURRENT_FILE_AMOUNT = 0;
static size_t CURRENT_STORAGE_SIZE = 0;

static icl_hash_t *clientFiles;
static pthread_mutex_t clientFilesMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct FileOpened {
    int *lock;
    char *path;
} FileOpened;

// Metodi della storage formato da mappa e lista
void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy);
void insert_storage(File file);
void print_storage();
void destroy_storage();

// Metodi della mappa
int add_client_files(int fileDescriptor, char *filePath, int flagLock);
FileOpened *get_client_files(Node *fdList, char *path);
void print_client_files();

// Metodi per gestire le richieste inviate dai client
int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock);
void *read_file(int fileDescriptor, char *filePath, int *bufferSize);
int write_file(int fileDescriptor, char *filePath,  char *fileContent);
int close_file(int fileDescriptor, char *filePath);

// ========================= METODI DELLO STORAGE =========================

void create_storage(size_t fileCapacity, size_t storageCapacity, int replacementPolicy) {
    STORAGE_FILE_CAPACITY = fileCapacity;
    STORAGE_CAPACITY = storageCapacity;

    inizialize_policy(replacementPolicy);

    clientFiles = icl_hash_create(100, NULL, int_compare);
}

void insert_storage(File file) {

    // Controllo se il file che sto inserendo è già presente nello storage
    File *tempFile = get_file_cache(file.filePath);
    // Se è presente, significa che devo aggiornare il contenuto del file
    if (tempFile != NULL) {       
        remove_file_cache(tempFile->filePath);

        CURRENT_FILE_AMOUNT--;
        CURRENT_STORAGE_SIZE -= *(tempFile->fileSize);
    }

    // Se il file da inserire supera la capacità massima dello storage, annullo l'inserimento
    if (*(file.fileSize) > STORAGE_CAPACITY) {
        fprintf(stderr, "ERRORE: il file non è stato inserito in quanto supera la capacità massima dello Storage.\n");
        return;
    }
    // Se il numero di file caricati sul server storage è maggiore della capacità massima, allora applico la politica di rimpiazzamento
    if (CURRENT_FILE_AMOUNT + 1 > STORAGE_FILE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta il massimo numero di File nello Storage.\n");
        
        File *fileToRemove = replacement_file_cache();

        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);

        free(fileToRemove->filePath);
        free(fileToRemove->fileContent);
		free(fileToRemove->fileSize);
        free(fileToRemove->fileLocked);
        free(fileToRemove->fileOpens);
        free(fileToRemove);
    }

    // Se la dimensione corrente + quella del file che sto andando a caricare supera la capacità massima, allora applico la politica di rimpiazzamtno
    while (CURRENT_STORAGE_SIZE + *(file.fileSize) > STORAGE_CAPACITY) {
        fprintf(stderr, "ATTENZIONE: raggiunta la dimensione massima dello Storage.\n");
        
        File *fileToRemove = replacement_file_cache();

        fprintf(stderr, "ATTENZIONE: %s è stato rimosso dallo Storage!\n", fileToRemove->filePath);

        free(fileToRemove->filePath);
        free(fileToRemove->fileContent);
		free(fileToRemove->fileSize);
        free(fileToRemove->fileLocked);
        free(fileToRemove->fileOpens);
        free(fileToRemove);
    }

    insert_file_cache(file);

    if (tempFile != NULL) {
        free(tempFile->filePath);
        free(tempFile->fileContent);
        free(tempFile->fileSize);
        free(tempFile->fileLocked);
        free(tempFile->fileOpens);
        free(tempFile);
    }

    CURRENT_FILE_AMOUNT++;
    CURRENT_STORAGE_SIZE += *(file.fileSize);
}

void print_storage() {
    print_client_files();
    print_cache();
}

void destroy_storage() {

    icl_entry_t *bucket, *curr;

    // Itero tutta la mappa e cancello il fileLock
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    FileOpened *fileOpened = (FileOpened *) temp->value;
                    free(fileOpened->path);
                    free(fileOpened->lock);
                    free(fileOpened);
                }
            }
            curr = curr->next;
        }
    }

    // Itero tutta la mappa e cancello i nodi
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    Node *freeNode = curr->data;
                    curr = curr->next;
                    free(freeNode);
                }
            }
            curr = curr->next;
        }
    }
  
    // Cancello tutte le chiavi della mappa
    icl_hash_destroy(clientFiles, free, NULL);
    destroy_cache();
}


// ========================= METODI DELLA MAPPA =========================

int add_client_files(int fileDescriptor, char *filePath, int flagLock) {
    
    // Acquisico la lock sulla mappa
    LOCK(&clientFilesMutex);
   
    // Cerco la lista del file descriptor passato per parametro
    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // Alloco la memoria
    FileOpened *fileOpened = (FileOpened *) malloc(sizeof(FileOpened));

    int *lock = (int *) malloc(sizeof(int));
    *lock = flagLock;
    
    char *path = malloc(sizeof(char) * (strlen(filePath) + 1));
    strcpy(path, filePath);

    fileOpened->lock = lock;
    fileOpened->path = path;

    // Se la lista è NULL, significa che l'utente ha fatto la sua prima open
    if (fdList == NULL) {      
        // Inizializzo la lista dell'utente
        Node *newFdList = NULL;

        // ALloco nell'heap il file descriptor dell'utente
        int *fd = (int *) malloc(sizeof(int));
        // Assegno alla variabile il file descriptor dell'utente passato per parametro
        *fd = fileDescriptor;

        add_tail(&newFdList, fileOpened);

        // Inserisco la prima entry nella mappa che ha come chiave, il fd dell'utente e come valore la lista di file path a cui ha fatto la open
        icl_hash_insert(clientFiles, fd, newFdList);

    // Controllo che il client non abbia richiesto una open su un file già presente nella lista
    } else if (get_client_files(fdList, fileOpened->path) == NULL) {
        // Se il file non è presente, allora lo aggiungo
        add_tail(&fdList, fileOpened);
    // Se il client ha richiesto la open su un file che aveva già aperto precedentemente
    } else {
        // Libero la memoria allocata
        free(path);
        free(lock);
        free(fileOpened);
        // Rilascio la lock
        UNLOCK(&clientFilesMutex);
        
        // Restituisco un messaggio di errore
        return -1;
    }
    
    // L'operazione è andata a buon fine
    UNLOCK(&clientFilesMutex);
    
    print_storage();
    
    return 1;
}

void remove_client_files(Node **fdList, char *filePath) {

    // Rimuovo il nodo dalla lista della mappa
    if (*fdList != NULL) {
		Node *currentFdList = *fdList;
		Node *precFdList = NULL;

		while (currentFdList != NULL) {
            FileOpened *fileOpened = (FileOpened *) currentFdList->value;
			if (strncmp(fileOpened->path, filePath, STRING_SIZE) == 0) {
				if (precFdList == NULL) {
					Node *tempNode = *fdList;
                    *fdList = (*fdList)->next;
					free(tempNode);  
					return;
				} else {
					Node *tempNode = *fdList;
					precFdList->next = currentFdList->next;
					free(tempNode);
					return;
				}
			}
			precFdList = currentFdList;
			currentFdList = currentFdList->next;
		}
	}
}

FileOpened *get_client_files(Node *fdList, char *path) {
	for (; fdList != NULL; fdList = fdList->next) {
		FileOpened *fileOpened = (FileOpened *) fdList->value;        
        if (strncmp(fileOpened->path, path, STRING_SIZE) == 0) {
			return fileOpened;
		}
	}
	return NULL;
}

void print_client_files() {
    icl_entry_t *bucket, *curr;

    printf("==== Client Files Map ====\n");
    
    for (int i = 0; i < clientFiles->nbuckets; i++) {
        bucket = clientFiles->buckets[i];
        for (curr = bucket; curr != NULL;) {
            if (curr->key) {
                int *fd = curr->key;
                printf("%d -> ", *fd);
                for (Node *temp = curr->data; temp != NULL; temp = temp->next) {
                    FileOpened *fileOpened = (FileOpened *) temp->value;
                    printf("[%s, %d] ", fileOpened->path, *(fileOpened->lock));
                }
                printf("\n");
            }
            curr = curr->next;
        }
    }

    printf("\n");
}

// ========================= METODI CHE GESTISOCNO LE RICHIESTE DEI CLIENT =========================

int open_file(int fileDescriptor, char *filePath, int flagCreate, int flagLock) {

    File *file = get_file_cache(filePath);

    if ((flagCreate == 1 && file != NULL) || (flagCreate == 0 && file == NULL)) {
        return 0;
    }

    if (flagCreate == 1 && file == NULL) {

        File newFile;
        newFile.filePath = filePath;
        newFile.fileContent = "Hello World!";
        
        size_t size = get_file_size(newFile);
        newFile.fileSize = &size;

        newFile.fileLocked = &flagLock;

        int opens = 1;
        newFile.fileOpens = &opens;

        insert_storage(newFile); 

        return add_client_files(fileDescriptor, newFile.filePath, flagLock);   
    }

    if (flagCreate == 0 && file != NULL) {

        int locked = get_file_lock(file);

        if (locked == 1) {
            return -2;
        }

        if (flagLock == 1 && get_files_opens(file) != 0) {
            return -3;
        }

        if (locked != flagLock) {
            set_file_lock(file, flagLock);
        }

        increase_file_opens(file);

        return add_client_files(fileDescriptor, file->filePath, flagLock);
    }

    return 0;
}

void *read_file(int fileDescriptor, char *filePath, int *bufferSize) {
    
    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    char *payload = NULL;

    // Se l'utente ha fatto la open sul file
    if (get_client_files(fdList, filePath) != NULL) {

        File *file = get_file_cache(filePath);

        int contentLength = strlen(file->fileContent) + 1;
        payload = malloc(contentLength);
        memcpy(payload, file->fileContent, contentLength);

        *bufferSize = contentLength;
    }
    
    UNLOCK(&clientFilesMutex);
    
    print_storage();
    
    return payload;
}

int write_file(int fileDescriptor, char *filePath, char *fileContent) {
    
    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    int response = 0;

    // Se l'utente ha fatto la open sul file
    if (get_client_files(fdList, filePath) != NULL) {
        File *file = get_file_cache(filePath);

        File tempFile;
        tempFile.filePath = file->filePath;
        tempFile.fileContent = fileContent;
        tempFile.fileLocked = file->fileLocked;
        tempFile.fileOpens = file->fileOpens;
        size_t size = get_file_size(tempFile);
        tempFile.fileSize = &size;

        insert_storage(tempFile); 

        response = 1;    
    }
    
    print_storage();

    UNLOCK(&clientFilesMutex);

    return response;
}

int remove_file(int fileDescriptor, char *filePath) {

    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    int response = 0;

    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;
        if (strncmp(fileOpened->path, filePath, STRING_SIZE) == 0) {
            if (*(fileOpened->lock) == 1) {

                File *file = get_file_cache(fileOpened->path);

                remove_file_cache(file->filePath);
                free(file->filePath);
                free(file->fileContent);
                free(file->fileSize);
                free(file->fileLocked);
                free(file->fileOpens);
                free(file);

                remove_client_files(&fdList, fileOpened->path);
                free(fileOpened->path);
                free(fileOpened->lock);
                free(fileOpened);
                
                response = 1;                
            } else {
                response = -1;
            }
            break;
        }
    }

    // Se la lista della mappa è uguale a null, significa che l'utente ha fatto la close sull'ultimo file che aveva aperto, quindi devo
    // cancellare la chiave dalla mappa e quando andrà ad eseguire una nuova open, riallocherò un nuovo fd. Altrimenti la insert alloca 4byte di memoria in più
    if (fdList == NULL) {
        icl_hash_delete(clientFiles, &fileDescriptor, free, NULL);
    }
    
    print_storage();
    
    UNLOCK(&clientFilesMutex);    

    return response;
}

int close_file(int fileDescriptor, char *filePath) {
    
    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    int response = 0;

    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;
        if (strncmp(fileOpened->path, filePath, STRING_SIZE) == 0) {
            File *file = get_file_cache(fileOpened->path);
            if (*(fileOpened->lock) == 1) {
                set_file_lock(file, 0);
            }
            decrease_file_opens(file);            

            remove_client_files(&fdList, fileOpened->path);
            free(fileOpened->path);
            free(fileOpened->lock);
            free(fileOpened);

            response = 1;

            break;
        }
    }

    // Se la lista della mappa è uguale a null, significa che l'utente ha fatto la close sull'ultimo file che aveva aperto, quindi devo
    // cancellare la chiave dalla mappa e quando andrà ad eseguire una nuova open, riallocherò un nuovo fd. Altrimenti la insert alloca 4byte di memoria in più
    if (fdList == NULL) {

        
        icl_hash_delete(clientFiles, &fileDescriptor, free, NULL);
    }
    
    print_storage();
    
    UNLOCK(&clientFilesMutex);    

    return response;
}

// Gestisco la disconnessione di un client
void disconnect_client(int fileDescriptor) {

    LOCK(&clientFilesMutex);

    Node *fdList = (Node *) icl_hash_find(clientFiles, &fileDescriptor);

    // cancello il valore associato ai nodi
    for (Node *curr = fdList; curr != NULL; curr = curr->next) {
        FileOpened *fileOpened = (FileOpened *) curr->value;

        File *file = get_file_cache(fileOpened->path);
        if (*(fileOpened->lock) == 1) {
            set_file_lock(file, 0);
        }
        decrease_file_opens(file);

        free(fileOpened->path);
        free(fileOpened->lock);
        free(fileOpened);
    }

    // cancello i nodi
    for (Node *curr = fdList; curr != NULL; ) {
        Node *temp = curr;
        curr = curr->next;
        free(temp);
    }

    // cancello la chiavi
    icl_hash_delete(clientFiles, &fileDescriptor, free, NULL);
    
    UNLOCK(&clientFilesMutex);
    
    print_storage();
}
