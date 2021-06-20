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
#include "../include/storage.h"

// - int openFile(const char* pathname, int flags)
// Richiesta di apertura o di creazione di un file. La semantica della openFile dipende dai flags passati come secondo
// argomento che possono essere O_CREATE ed O_LOCK. Se viene passato il flag O_CREATE ed il file esiste già
// memorizzato nel server, oppure il file non esiste ed il flag O_CREATE non è stato specificato, viene ritornato un
// errore. In caso di successo, il file viene sempre aperto in lettura e scrittura, ed in particolare le scritture possono
// avvenire solo in append. Se viene passato il flag O_LOCK (eventualmente in OR con O_CREATE) il file viene
// aperto e/o creato in modalità locked, che vuol dire che l’unico che può leggere o scrivere il file ‘pathname’ è il
// processo che lo ha aperto. Il flag O_LOCK può essere esplicitamente resettato utilizzando la chiamata unlockFile,
// descritta di seguito.
// Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene settato opportunamente.


void handlePacket(int packetID, int packetSize, char *payload, int fileDescriptor) {

    int pathLength, contentLength, response, flagCreate, flagLock, n;
    char *path;

    switch (packetID) {

        case OPEN_FILE:
            pathLength = *((int *) payload);
            path = payload + sizeof(int);
            flagCreate = *((int *) (payload + sizeof(int) + pathLength));
            flagLock = *((int *) (payload + sizeof(int) + pathLength + sizeof(int)));

            response = open_file(fileDescriptor, path, flagCreate, flagLock);
            write(fileDescriptor, &response, sizeof(int));

            printf("SERVER: Ricevuta una richiesta di open sul file \"%s\"\nSERVER: Risposta: %d\n", path, response);
            
            break;

        case READ_FILE:
            path = payload;

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
            n = *((int *) payload);
            int bufferSize = 0;

            char *test = read_n_file(n, &bufferSize);

            write(fileDescriptor, &bufferSize, sizeof(int));
            printf("Dimensione Buffer Size %d\n", bufferSize);
            write(fileDescriptor, test, bufferSize);

            free(test);

            break;

        case WRITE_FILE:
            pathLength = *((int *) payload);
            path = payload + sizeof(int);
            contentLength = *((int *) (payload + sizeof(int) + pathLength));
            content = (payload + sizeof(int) + pathLength + sizeof(int));

            response = write_file(fileDescriptor, path, content);
            write(fileDescriptor, &response, sizeof(int));
           
            printf("SERVER: Ricevuta una richiesta di write sul file \"%s\"\nSERVER: Risposta: %d\n", path, response);
            break;

        case APPEND_TO_FILE:

            break;

        case CLOSE_FILE:       
            path = payload;

            response = close_file(fileDescriptor, path);
            write(fileDescriptor, &response, sizeof(int));

            printf("SERVER: Ricevuta una richiesta di close sul file \"%s\"\nSERVER: Risposta: %d\n", path, response);
            break;
        
        case REMOVE_FILE:
            pathLength = *((int *) payload);
            path = payload + 4;

            response = remove_file(fileDescriptor, path);
            
            switch (response) {
                case 1:
                    write(fileDescriptor, "Remove eseguita con successo!", 100);
                    break;
                case 0:
                    write(fileDescriptor, "Devi prima eseguire la open su quel file!", 100);
                    break;
                case -1:
                    write(fileDescriptor, "Per rimuovere il file devi prima settare il flag di Lock a 1!", 100);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;       
    }

}
 
void handleDisconnect(int fileDescriptor) {    
    disconnect_client(fileDescriptor);      
}
