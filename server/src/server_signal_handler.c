#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include "../include/server_signal_handler.h"
#include "../include/server_packet_handler.h"

void *handle_signal(void *handlerArgument) {

    sigset_t blockMask;
    int pipeHandleConnection[2];
    SignalHandlerArg *argument = (SignalHandlerArg *)handlerArgument;

    blockMask = argument->blockMask;
    pipeHandleConnection[0] = argument->pipeHandleConnection[0];
    pipeHandleConnection[1] = argument->pipeHandleConnection[1];

    CONNECTION = 1;

    int signal;
    sigwait(&blockMask, &signal);
    switch (signal) {
        case SIGINT:
            ;
            char ch[4] = "ciao";

            if (write(pipeHandleConnection[1], &ch, 4) == -1) {
                perror("ERRORE PIPE\n");
            }

            CONNECTION = 0;
            broadcast();
            
            printf("SIGINT\n");

            break;
        case SIGQUIT:
            printf("SIGQUIT\n");
            break;
        case SIGHUP:
            printf("SIGHUP\n");
            break;    
        default:
            break;
    }
}