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

#include "../include/server_packet_handler.h"
#include "../include/server_storage.h"

void handlePacket(int packetID, int packetSize, char *payload, int fileDescriptor) {

    int pathLength, contentLength, response, flagCreate, flagLock;
    char *path;

    switch (packetID) {

        case OPEN_FILE:
            pathLength = *((int *) payload);
            path = payload + sizeof(int);
            flagCreate = *((int *) (payload + sizeof(int) + pathLength));
            flagLock = *((int *) (payload + sizeof(int) + pathLength + sizeof(int)));

            printf("SERVER: Ricevuta una richiesta di open sul file \"%s\"\n", path);

            response = open_file(fileDescriptor, path, flagCreate, flagLock);
            write(fileDescriptor, &response, sizeof(int));
            break;

        case READ_FILE:
            path = payload;
            
            printf("SERVER: Ricevuta una richiesta di read sul file \"%s\"\n", path);

            contentLength = 0;
            char *content = read_file(fileDescriptor, path, &contentLength);

            // Se l'utente ha eseguito la open sul file
            if (content != NULL) {
                char *buffer = malloc(sizeof(int) + contentLength);

                memcpy(buffer, &contentLength, sizeof(int));
                memcpy(buffer + sizeof(int), content, contentLength);

                write(fileDescriptor, buffer, (sizeof(int) + contentLength));

                free(content);
                free(buffer);
            // Se l'utente non ha eseguito la open sul file
            } else {
                write(fileDescriptor, &contentLength, sizeof(int));
            }

            break;

        case READ_N_FILES:
            // n = *((int *) payload);
            // int bufferSize = 0;

            // char *test = read_n_file(n, &bufferSize);

            // write(fileDescriptor, &bufferSize, sizeof(int));
            // write(fileDescriptor, test, bufferSize);

            // free(test);

            break;

        case WRITE_FILE:
            pathLength = *((int *) payload);
            path = payload + sizeof(int);
            contentLength = *((int *) (payload + sizeof(int) + pathLength));
            content = (payload + sizeof(int) + pathLength + sizeof(int));

            printf("SERVER: Ricevuta una richiesta di write sul file \"%s\"\n", path);

            response = write_file(fileDescriptor, path, content);
            write(fileDescriptor, &response, sizeof(int));
            break;

        case APPEND_TO_FILE:

            break;

        case CLOSE_FILE:       
            path = payload;

            printf("SERVER: Ricevuta una richiesta di close sul file \"%s\"\n", path);
        
            response = close_file(fileDescriptor, path);
            write(fileDescriptor, &response, sizeof(int)); 
            break;
        
        case REMOVE_FILE:
            pathLength = *((int *) payload);
            path = payload + sizeof(int);

            printf("SERVER: Ricevuta una richiesta di remove sul file \"%s\"\n", path);

            response = remove_file(fileDescriptor, path);
            write(fileDescriptor, &response, sizeof(int));
            break;

        default:
            fprintf(stderr, "ERRORE: Ricevuta una richiesta inesistente!");
            break;       
    }

}
 
void handleDisconnect(int fileDescriptor) {    
    disconnect_client(fileDescriptor);      
}
