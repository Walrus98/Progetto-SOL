#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/pthread_utils.h"
#include "../include/server_network.h"
#include "../include/server_network_dispatcher.h"
#include "../include/server_network_worker.h"

static pthread_t create_thread_dispatcher();
static pthread_t *create_thread_pool(size_t poolSize);

static int pfd[2];

void create_connection(size_t poolSize) {
    
    if (pipe(pfd) == -1) {
        printf("ERRORE !!\n");
    }
    // close(pfd[1]);
    // int l = read(pfd[0], msg, 4);
    // close(pfd[0]);    

    pthread_t threadDispatcher = create_thread_dispatcher();
    pthread_t *threadWorker = create_thread_pool(poolSize);

    JOIN(threadDispatcher);

    for (int i = 0; i < poolSize; i++) {
        JOIN(threadWorker[i]);
    }
    free(threadWorker);
}

static pthread_t create_thread_dispatcher() {
    pthread_t threadDispatcher;

    if (pthread_create(&threadDispatcher, NULL, &dispatch_connection, (void *) pfd) != 0) {
        fprintf(stderr, "ERRORE: impossibile creare il Thread Dispatcher\n");
        exit(EXIT_FAILURE);
    }
    return threadDispatcher;
}

static pthread_t *create_thread_pool(size_t poolSize) {

    pthread_t *threadPool;

    if ((threadPool = (pthread_t *) malloc(sizeof(pthread_t) * poolSize)) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione della Thread Pool");
		exit(errno);
	}

    for (int i = 0; i < poolSize; i++) {
        if (pthread_create(&threadPool[i], NULL, &handle_connection, (void *) pfd) != 0) {
            fprintf(stderr, "ERRORE: impossibile creare la Thread Pool\n");
            exit(EXIT_FAILURE);
        }
    }
    return threadPool;
}
