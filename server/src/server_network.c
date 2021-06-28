#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "../include/pthread_utils.h"
#include "../include/server_network.h"
#include "../include/server_signal_handler.h"
#include "../include/server_network_dispatcher.h"
#include "../include/server_network_worker.h"
#include "../include/server_network_handler.h"

static pthread_t create_thread_signal(int pipeHandleConnection[], sigset_t blockMask);
static pthread_t create_thread_dispatcher(int pipeHandleConnection[], int pfd[]);
static pthread_t *create_thread_pool(int pfd[], size_t poolSize);

void create_connection(size_t poolSize) {

    // Variabile globale per gestire la condizione di terminazione dei thread
    CONNECTION = 1;

    // Creo la maschera utilazzata per la gestione dei segnali
    sigset_t blockMask;
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGINT);
    sigaddset(&blockMask, SIGQUIT);
    sigaddset(&blockMask, SIGHUP);
    pthread_sigmask(SIG_SETMASK, &blockMask, NULL);

    // Pipe utilizzata per reinserire il file descriptor all'interno del Thread Dispatcher dopo che il Thread Worker ha terminato il task
    int pipeHandleClient[2];

    // Pipe utilizzata per terminare il Dispatcher, è usata da il Thread Signal Handler e il Thread Dispatcher
    int pipeHandleConnection[2];
    
    // Inizializzo la pipe
    if (pipe(pipeHandleConnection) == -1) {
        fprintf(stderr, "ERRORE: impossibile creare tra il Thread Signal Handler e il Thread Dispatcher\n");
        exit(EXIT_FAILURE);
    }

    // Inizializzo la pipe
    if (pipe(pipeHandleClient) == -1) {
        fprintf(stderr, "ERRORE: impossibile creare tra il Thread Dsipatcher e i Thread Worker\n");
        exit(EXIT_FAILURE);
    }

    // Creo i thread necessari per il corretto funzionamento del server
    pthread_t threadSignalHandler = create_thread_signal(pipeHandleConnection, blockMask);
    pthread_t threadDispatcher = create_thread_dispatcher(pipeHandleConnection, pipeHandleClient);
    pthread_t *threadWorker = create_thread_pool(pipeHandleClient, poolSize);

    // Eseguo la JOIN su i thread creati. In questo modo il thread main termina solo quando tutti gli altri thread sono terminati
    JOIN(threadSignalHandler);
    printf("SERVER: Termino l'esecuzione del Thread Signal Handler\n");
    JOIN(threadDispatcher);
    printf("SERVER: Termino l'esecuzione del Thread Dispatcher\n");
    for (int i = 0; i < poolSize; i++) {
        JOIN(threadWorker[i]);
        printf("SERVER: Termino l'esecuzione del Thread Worker\n");
    }
    // Libero la memoria
    free(threadWorker);
}

// Creo il Thread Signal Handler
static pthread_t create_thread_signal(int pipeHandleConnection[], sigset_t blockMask) {

    // Creo una struct per passare gli argomenti alla funzione pthread_create
    SignalHandlerArg handlerArgument;
    // Pipe per svegliare il Thread Dispatcher quando deve terminare
    handlerArgument.pipeHandleConnection[0] = pipeHandleConnection[0];
    handlerArgument.pipeHandleConnection[1] = pipeHandleConnection[1];
    // Maschera dei segnali
    handlerArgument.blockMask = blockMask;
    
    // Creo il Thread Signal Handler
    pthread_t threadSignalHandler;
    if (pthread_create(&threadSignalHandler, NULL, &handle_signal, &handlerArgument) != 0) {
        fprintf(stderr, "ERRORE: impossibile creare il Thread Signal Handler\n");
        exit(EXIT_FAILURE);
    }

    // Restituisco il thread appena creato
    return threadSignalHandler;
}

// Creo il Thread Dispatcher
static pthread_t create_thread_dispatcher(int pipeHandleConnection[], int pipeHandleClient[]) {

    // Creo una struct per passare gli argomenti alla funzione pthread_create
    DispatcherArg dispatcherArgument;
    // Pipe per svegliare il Thread Dispatcher quando deve terminare
    dispatcherArgument.pipeHandleConnection[0] = pipeHandleConnection[0];
    dispatcherArgument.pipeHandleConnection[1] = pipeHandleConnection[1];
    // Pipe per ricevere il file descriptor del client quando il Thread Worker ha terminato il Task
    dispatcherArgument.pipeHandleClient[0] = pipeHandleClient[0];
    dispatcherArgument.pipeHandleClient[1] = pipeHandleClient[1];

    // Creo il Thread Dispatcher
    pthread_t threadDispatcher;
    if (pthread_create(&threadDispatcher, NULL, &dispatch_connection, &dispatcherArgument) != 0) {
        fprintf(stderr, "ERRORE: impossibile creare il Thread Dispatcher\n");
        exit(EXIT_FAILURE);
    }

    // Restituisco il thread appena creato
    return threadDispatcher;
}

static pthread_t *create_thread_pool(int pipeHandleClient[], size_t poolSize) {

    // Creo una pool di thread
    pthread_t *threadPool;
    if ((threadPool = (pthread_t *) malloc(sizeof(pthread_t) * poolSize)) == NULL) {
        perror("ERRORE: impossibile allocare la memoria richiesta per la creazione della Thread Pool");
		exit(errno);
	}

    // Inizializzo i Thread appena creati
    for (int i = 0; i < poolSize; i++) {
        if (pthread_create(&threadPool[i], NULL, &handle_connection, (void *) pipeHandleClient) != 0) {
            fprintf(stderr, "ERRORE: impossibile creare la Thread Pool\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Restituisco la pool di Thread Worker
    return threadPool;
}
