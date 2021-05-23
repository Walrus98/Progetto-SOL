#ifndef SERVER_SIGNAL_HANDLER_H
#define SERVER_SIGNAL_HANDLER_H

typedef struct SignalHandlerArg {
    int pipeHandleConnection[2];
    sigset_t blockMask;
} SignalHandlerArg;

void *handle_signal(void *handlerArgument);

#endif