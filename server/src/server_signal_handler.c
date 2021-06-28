#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include "../include/server_signal_handler.h"
#include "../include/server_network_handler.h"
#include "../../core/include/utils.h"

void *handle_signal(void *handlerArgument) {

    sigset_t blockMask;
    int pipeHandleConnection[2];

    // Prendo gli argomenti passati dalla funzione
    SignalHandlerArg *argument = (SignalHandlerArg *) handlerArgument;

    // Prendo la maschera creata precedentemente
    blockMask = argument->blockMask;
    // Prendo la pipe utilizzata per comunicare con il Thread Dispatcher
    pipeHandleConnection[0] = argument->pipeHandleConnection[0];
    pipeHandleConnection[1] = argument->pipeHandleConnection[1];

    // Mi metto in attesa di ricevere un segnale
    int signal;
    sigwait(&blockMask, &signal);

    char message[10];

    // Se il segnale ricevuto è un SIGINT o SIGQUIT
    if (signal == SIGINT || signal == SIGQUIT) {
        // Inizializzo message con force-stop
        strncpy(message, "force-stop", 10);
        printf("\nSERVER: Ricevuto un segnale di SIGNINT o SIGQUIT\n");
    } else {
        // Altrimenti inizializzo message con stop
        strncpy(message, "stop", 10);
        printf("\nSERVER: Ricevuto un segnale di SIGHUP\n");
    }

    // Tramite pipe invio al Thread Dispatcher il contenuto di message, in base a quello il 
    // Thread Dispatcher capisce il modo in cui deve terminare
    if (writen(pipeHandleConnection[1], &message, 10) == -1) {
        perror("ERRORE PIPE\n");
        exit(errno);
    }
    
    // Chiudo la scrittura della pipe
    close(pipeHandleConnection[1]);

    return NULL;
}