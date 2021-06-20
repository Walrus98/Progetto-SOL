#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include "../include/server_signal_handler.h"
#include "../include/server_network_handler.h"

void *handle_signal(void *handlerArgument) {

    sigset_t blockMask;
    int pipeHandleConnection[2];
    SignalHandlerArg *argument = (SignalHandlerArg *) handlerArgument;

    blockMask = argument->blockMask;
    pipeHandleConnection[0] = argument->pipeHandleConnection[0];
    pipeHandleConnection[1] = argument->pipeHandleConnection[1];

    int signal;
    sigwait(&blockMask, &signal);

    char message[10];

    if (signal == SIGINT || signal == SIGQUIT) {
        strncpy(message, "force-stop", 10);

        printf("\nSERVER: Ricevuto un segnale di SIGNINT o SIGQUIT\n");
    } else {
        strncpy(message, "stop", 10);

        printf("\nSERVER: Ricevuto un segnale di SIGHUP\n");
    }

    if (write(pipeHandleConnection[1], &message, 10) == -1) {
        perror("ERRORE PIPE\n");
    }
    
    close(pipeHandleConnection[1]);

    return NULL;
}

// if (signal == SIGINT || signal == SIGQUIT) {

//     // CONNECTION = 0;

//     // broadcast();

//     char message[10] = "force-stop";

//     if (write(pipeHandleConnection[1], &message, 10) == -1) {
//         perror("ERRORE PIPE\n");
//     }
            
//     printf("SIGINT o SIGQUIT\n");

// } else if (signal == SIGHUP) { 

//     char message[10] = "stop";
    
//     if (write(pipeHandleConnection[1], &message, 10) == -1) {
//         perror("ERRORE PIPE\n");
//     }
    
//     printf("SIGHUP\n");
// }
