#ifndef SERVER_NETWORK_DISPATCHER_H
#define SERVER_NETWORK_DISPATCHER_H

typedef struct DispatcherArg {
    int pipeHandleConnection[2];
    int pipeHandleClient[2];
} DispatcherArg;

void *dispatch_connection(void *dispatcherArgument);

#endif