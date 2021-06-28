#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../include/server_network_worker.h"
#include "../include/server_network_handler.h"
#include "../include/server_packet_handler.h"
#include "../../core/include/utils.h"

#define READ_PACKET(buffer, pipeTask, fd, size)                     \
    nread = readn(fd, buffer, size);                                \
    if (nread == 0 || nread == -1)                                  \
    {                                                               \
        int connectedClients = -1;                                  \
        if (writen(pipeTask, &connectedClients, sizeof(int)) == -1) \
            exit(errno);                                            \
        handleDisconnect(fd);                                       \
        close(fd);                                                  \
        continue;                                                   \
    }

void *handle_connection(void *pipeHandleClient) {

    // Prendo la pipe passata per argomento
    int *pipeTask = (int *) pipeHandleClient;

    char *packetHeader = malloc(sizeof(int) * 2);
    char *packetPayload = NULL;
    int nread;

    while (CONNECTION == 1) {
        // Prendo il task dalla lista di fd
        int fileDescriptor = popPacket();
        
        // Se il fd è -1, allora devo terminare l'esecuzione del thread
        if (fileDescriptor == -1) {
            break;
        }

        // leggo l'header del pacchetto
        READ_PACKET(packetHeader, pipeTask[1], fileDescriptor, sizeof(int) * 2);
        int packetID = *((int *) packetHeader);
        int packetSize = *((int *) packetHeader + 1);

        // Alloco la dimensione del buffer payload
        if ((packetPayload = (char *) malloc(packetSize)) == NULL) {
            perror("ERRORE: Impossibile allocare la memoria richiesta");
            exit(errno);
        }

        // leggo il paylod del pacchetto
        READ_PACKET(packetPayload, pipeTask[1], fileDescriptor, packetSize);
        
        // Gestisco la richiesta in base al pacchetto ricevuto
        handlePacket(packetID, packetSize, packetPayload, fileDescriptor);

        // Attraverso la pipe mando indietro il fd al Thread Dispatcher
        if (writen(pipeTask[1], &fileDescriptor, sizeof(int)) == -1) {
            exit(errno);
        }     
  
        // Libero il payload allocato precedentemente
        free(packetPayload);
    }

    close(pipeTask[1]);
    free(packetHeader);
    
    return NULL;
}