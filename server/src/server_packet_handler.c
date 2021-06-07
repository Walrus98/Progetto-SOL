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

    switch (packetID) {

        case OPEN_FILE:
            ;
            fileLength = *((int *) payload);
            filePath = payload + 4;
            int flagCreate = *((int *) (payload + 4 + fileLength));
            int flagLock = *((int *) (payload + 4 + fileLength + 4));

            // printf("Header ID -> %d\n", packetID);
            // printf("Header Size -> %d\n", packetSize);
            // printf("Name Length -> %d\n", fileLength);
            // printf("Name -> %s\n", filePath);
            // printf("Flag Create -> %d\n", flagCreate);
            // printf("Flag Lock -> %d\n", flagLock);

            int response = open_file(fileDescriptor, filePath, flagCreate, flagLock);

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

            int payloadLength = 0;
            char *payloadResponse = read_file(fileDescriptor, filePath, &payloadLength);

            int id = READ_FILE;
            char *headerResponse = malloc(sizeof(int) * 2);
            memcpy(headerResponse, &id, sizeof(int));
            memcpy(headerResponse + sizeof(int), &payloadLength, sizeof(int));
    
            write(fileDescriptor, headerResponse, sizeof(int) * 2);
            write(fileDescriptor, payloadResponse, payloadLength);

            free(headerResponse);
            free(payloadResponse);
            
            break;

        case READ_N_FILES:
            break;

        case WRITE_FILE:
            ;
            fileLength = *((int *) payload);
            filePath = payload + 4;

            char fileContent[7] = "BAUBAB";

            write_file(fileDescriptor, filePath, fileContent);

            write(fileDescriptor, "OK!", 100); 

            break;

        case APPEND_TO_FILE:

            break;

        case CLOSE_FILE:            
            ;
            fileLength = *((int *) payload);
            filePath = payload + 4;

            close_file(fileDescriptor, filePath);

            break;
        
        case REMOVE_FILE:

            break;

        default:
            break;       
    }

}
 
void handleDisconnect(int fileDescriptor) {    
    disconnect_client(fileDescriptor);      
}
