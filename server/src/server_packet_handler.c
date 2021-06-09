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

    int fileLength;
    char *filePath;
    int contentLength;
    int response;
    int flagCreate;
    int flagLock;


    switch (packetID) {

        case OPEN_FILE:
            fileLength = *((int *) payload);
            filePath = payload + sizeof(int);
            flagCreate = *((int *) (payload + sizeof(int) + fileLength));
            flagLock = *((int *) (payload + sizeof(int) + fileLength + sizeof(int)));

            // printf("Header ID -> %d\n", packetID);
            // printf("Header Size -> %d\n", packetSize);
            // printf("Name Length -> %d\n", fileLength);
            // printf("Name -> %s\n", filePath);
            // printf("Flag Create -> %d\n", flagCreate);
            // printf("Flag Lock -> %d\n", flagLock);

            response = open_file(fileDescriptor, filePath, flagCreate, flagLock);

            switch (response) {
                case 1:
                    write(fileDescriptor, "Apertura del File eseguita con successo!", 100);
                    break;
                case 0:
                    write(fileDescriptor, "Impossibile aprire il File!", 100);
                    break;
                case -1:
                    write(fileDescriptor, "Impossibile eseguire open multiple sullo stesso file!", 100);
                    break;
                case -2:
                    write(fileDescriptor, "Impossibile eseguire la open sul file richiesto perché è in stato di Locked", 100); 
                    break;
                case -3:
                    write(fileDescriptor, "Impossibile eseguire la open sul file con il flag di Lock a 1 perché in questo momento è aperto da altri utenti", 100); 
                    break;
                default:
                    break;
            }
            break;

        case READ_FILE:
            ;
            fileLength = *((int *) payload);
            filePath = payload + 4;

            contentLength = 0;
            char *content = read_file(fileDescriptor, filePath, &contentLength);

            // Se l'utente non ha eseguito la open sul file
            if (content == NULL) {
                content = (char *) malloc(sizeof(char) * 100);
                strncpy(content, "Devi prima eseguire la open su quel file!", 100);
                contentLength = 100;
            }
            char *buffer = malloc(sizeof(int) + contentLength);

            memcpy(buffer, &contentLength, sizeof(int));
            memcpy(buffer + sizeof(int), content, contentLength);

            write(fileDescriptor, buffer, (sizeof(int) + contentLength));

            free(content);
            free(buffer);

            break;

        case READ_N_FILES:
            ;

            int n = 2;
            int bufferSize = 0;

            char *test = read_n_file(n, &bufferSize);

            write(fileDescriptor, &bufferSize, sizeof(int));
            write(fileDescriptor, test, bufferSize);

            free(test);

            break;

        case WRITE_FILE:
            ;
            fileLength = *((int *) payload);
            filePath = payload + sizeof(int);

            contentLength = *((int *) (payload + sizeof(int) + fileLength));
            content = (payload + sizeof(int) + fileLength + sizeof(int));

            response = write_file(fileDescriptor, filePath, content);

            if (response == 1) {
                write(fileDescriptor, "Write eseguita con successo!", 100); 
            } else {
                write(fileDescriptor, "Devi prima eseguire la open su quel file!", 100); 
            }

            break;

        case APPEND_TO_FILE:

            break;

        case CLOSE_FILE:            
            ;
            fileLength = *((int *) payload);
            filePath = payload + 4;

            response = close_file(fileDescriptor, filePath);

            if (response == 1) {
                write(fileDescriptor, "Close eseguita con successo!", 100); 
            } else {
                write(fileDescriptor, "Devi prima eseguire la open su quel file!", 100); 
            }
            
            break;
        
        case REMOVE_FILE:
            ;
            fileLength = *((int *) payload);
            filePath = payload + 4;

            response = remove_file(fileDescriptor, filePath);
            
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
