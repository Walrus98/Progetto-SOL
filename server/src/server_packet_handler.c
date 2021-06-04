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
    switch (packetID) {

        case OPEN_FILE:

            ;
            int fileLength = *((int *) payload);
            char *filePath = payload + 4;
            int flagCreate = *((int *) (payload + 4 + fileLength));
            int flagLock = *((int *) (payload + 4 + fileLength + 4));

            // printf("Header ID -> %d\n", packetID);
            // printf("Header Size -> %d\n", packetSize);
            // printf("Name Length -> %d\n", fileLength);
            // printf("Name -> %s\n", filePath);
            // printf("Flag Create -> %d\n", flagCreate);
            // printf("Flag Lock -> %d\n", flagLock);

            int response = openFile(fileDescriptor, filePath, flagCreate, flagLock);

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
                default:
                    break;
            }
            break;
    }
}
 
void handleDisconnect(int fileDescriptor) {
    remove_client_files(fileDescriptor);        
}
