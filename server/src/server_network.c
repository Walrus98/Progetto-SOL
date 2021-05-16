#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/pthread_utils.h"
#include "../include/server_network.h"
#include "../include/server_network_dispatcher.h"
#include "../include/server_network_worker.h"

static pthread_t create_thread_dispatcher();
static void create_thread_pool(int poolSize);

void create_connection() {

    pthread_t threadDispatcher = create_thread_dispatcher();
    create_thread_pool(20);

    JOIN(threadDispatcher);
}

static pthread_t create_thread_dispatcher() {
    pthread_t threadDispatcher;
    if (pthread_create(&threadDispatcher, NULL, &dispatch_connection, NULL) != 0) {
        fprintf(stderr, "ERRORE: impossibile creare il Thread Dispatcher\n");
        exit(EXIT_FAILURE);
    }
    return threadDispatcher;
}

static void create_thread_pool(int poolSize) {

    // pthread_t threadPool;

    // if (pthread_create(&threadPool, NULL, &handle_connection, NULL) != 0) {
    //     fprintf(stderr, "ERRORE: impossibile creare la Thread Pool\n");
    //     exit(EXIT_FAILURE);
    // }

    // JOIN(threadPool);

    // pthread_t threadPool[3];

    // for (int i = 0; i < poolSize; i++) {
    //     if (pthread_create(&threadPool[i], NULL, &handle_connection, NULL) != 0) {
    //         fprintf(stderr, "ERRORE: impossibile creare la Thread Pool\n");
    //         exit(EXIT_FAILURE);
    //     }
    // }
}
