#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../include/client_network.h"

#define UNIX_PATH_MAX 108 
#define BUFFER_RESPONSE_SIZE 100

/**
 * Viene aperta una connessione AF_UNIX al socket file sockname. Se il server non accetta immediatamente la
 * richiesta di connessione, la connessione da parte del client viene ripetuta dopo ‘msec’ millisecondi e fino allo
 * scadere del tempo assoluto ‘abstime’ specificato come terzo argomento. Ritorna 0 in caso di successo, -1 in caso
 * di fallimento, errno viene settato opportunamente.
 **/

static int SERVER_SOCKET;

int openConnection(const char* sockname, int msec, const struct timespec abstime) {
    
    struct sockaddr_un sa;
    strncpy(sa.sun_path, sockname, UNIX_PATH_MAX);
    sa.sun_family = AF_UNIX;

    SERVER_SOCKET = socket(AF_UNIX, SOCK_STREAM, 0);

    // struct timespec timeWait;
    // set_timespec_from_msec(msec, &timeWait);

    while (connect(SERVER_SOCKET, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        if (errno == ENOENT) {
            /* sock non esiste */
            // nanosleep(timeWait); 
        } else
            exit(EXIT_FAILURE);
    }

    return 0;
}

int closeConnection(const char* sockname) {
    close(SERVER_SOCKET);
}

/**
 * Richiesta di apertura o di creazione di un file. La semantica della openFile dipende dai flags passati come secondo
 * argomento che possono essere O_CREATE ed O_LOCK. Se viene passato il flag O_CREATE ed il file esiste già
 * memorizzato nel server, oppure il file non esiste ed il flag O_CREATE non è stato specificato, viene ritornato un
 * errore. In caso di successo, il file viene sempre aperto in lettura e scrittura, ed in particolare le scritture possono
 * avvenire solo in append. Se viene passato il flag O_LOCK (eventualmente in OR con O_CREATE) il file viene
 * aperto e/o creato in modalità locked, che vuol dire che l’unico che può leggere o scrivere il file ‘pathname’ è il
 * processo che lo ha aperto. Il flag O_LOCK può essere esplicitamente resettato utilizzando la chiamata unlockFile,
 * descritta di seguito.
 * Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene settato opportunamente. 
 **/

int openFile(const char* pathname, int flags) {

    FILE *file = NULL;
    if ((file = fopen(pathname, "r")) == NULL) {
        perror("ERRORE: apertura del file");
        exit(errno);
    }

    printf("Invio una open di %s\n", pathname);

    // char absolutePath[1000];
    // realpath("prova/test2", absolutePath);

    int id = OPEN_FILE;
    int pathLength = strlen(pathname) + 1;
    int ocreate = 1;
    int olock = 0;
    int payloadLength = sizeof(int) + pathLength + sizeof(int) + sizeof(int);

    // Header
    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));

    // Payload
    char *payload = malloc(payloadLength);
    memcpy(payload, &pathLength, sizeof(int));
    memcpy(payload + sizeof(int), pathname, pathLength);
    memcpy(payload + sizeof(int) + pathLength, &ocreate, sizeof(int));
    memcpy(payload + sizeof(int) + pathLength + sizeof(int), &olock, sizeof(int));

    write(SERVER_SOCKET, header, sizeof(int) * 2);
    write(SERVER_SOCKET, payload, payloadLength);

    // Response
    char *response = malloc(sizeof(char) * 100);

    read(SERVER_SOCKET, response, BUFFER_RESPONSE_SIZE);
    printf("Messaggio ricevuto: %s\n\n", response);
    
    free(header);
    free(payload);
    free(response);

    if (fclose(file) != 0) {
        perror("ERRORE: chiusura del file");
        exit(EXIT_FAILURE);
    }
}

/**
 * Legge tutto il contenuto del file dal server (se esiste) ritornando un puntatore ad un'area allocata sullo heap nel
 * parametro ‘buf’, mentre ‘size’ conterrà la dimensione del buffer dati (ossia la dimensione in bytes del file letto). In
 * caso di errore, ‘buf‘e ‘size’ non sono validi. Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene
 * settato opportunamente.
 **/
int readFile(const char* pathname, void** buf, size_t* size) {
    
    FILE *file = NULL;
    if ((file = fopen(pathname, "r")) == NULL) {
        perror("ERRORE: apertura del file");
        exit(errno);
    }

    int id = READ_FILE;
    int pathLength = strlen(pathname) + 1;
    int payloadLength = sizeof(int) + pathLength;

    // Header
    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // Payload
    char *payload = malloc(payloadLength);
    // memcpy(payload, &pathLength, sizeof(int));
    memcpy(payload, pathname, pathLength);

    write(SERVER_SOCKET, header, sizeof(int) * 2);
    write(SERVER_SOCKET, payload, payloadLength);

    // Response Header
    char *headerResponse = malloc(sizeof(int));
    read(SERVER_SOCKET, headerResponse,  sizeof(int));
    int packetSize = *((int *) headerResponse);

    // Response Payload
    char *payloadResponse = malloc(sizeof(packetSize));
    read(SERVER_SOCKET, payloadResponse, packetSize);

    printf("Client got : %s\n\n", payloadResponse);

    if (fclose(file) != 0) {
        perror("ERRORE: chiusura del file");
        exit(EXIT_FAILURE);
    }

}

int readNFiles(int N, const char* dirname) {

}

/**
 * Scrive tutto il file puntato da pathname nel file server. Ritorna successo solo se la precedente operazione,
 * terminata con successo, è stata openFile(pathname, O_CREATE| O_LOCK). Se ‘dirname’ è diverso da NULL, il
 * file eventualmente spedito dal server perchè espulso dalla cache per far posto al file ‘pathname’ dovrà essere
 * scritto in ‘dirname’; Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene settato opportunamente.
 **/
int writeFile(const char* pathname, const char* dirname) {

    FILE *file = NULL;
    if ((file = fopen(pathname, "r")) == NULL) {
        perror("ERRORE: apertura del file");
        exit(errno);
    }

    printf("Invio una write di %s\n", pathname);

    fseek(file, 0, SEEK_END);
    int textLength = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(textLength);
    fread(content, 1, textLength, file); 

    int id = WRITE_FILE;
    int pathLength = strlen(pathname) + 1;
    int contentLength = strlen(content) + 1;
    int payloadLength = sizeof(int) + pathLength + sizeof(int) + contentLength;

    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // payload
    char *payload = malloc(payloadLength);
    char *currentPosition = payload;

    memcpy(currentPosition, &pathLength, sizeof(int));
    currentPosition += sizeof(int);
    memcpy(currentPosition, pathname, pathLength);
    currentPosition += pathLength;
    memcpy(currentPosition, &contentLength, pathLength);
    currentPosition += sizeof(int);
    memcpy(currentPosition, content, contentLength);

    write(SERVER_SOCKET, header, sizeof(int) * 2);
    write(SERVER_SOCKET, payload, payloadLength);
    
    // Response
    char *response = malloc(sizeof(char) * 100);

    read(SERVER_SOCKET, response, BUFFER_RESPONSE_SIZE);
    printf("Messaggio ricevuto: %s\n\n", response);
    
    free(content);
    free(header);
    free(payload);
    free(response);

    if (fclose(file) != 0) {
        perror("ERRORE: chiusura del file");
        exit(EXIT_FAILURE);
    }
}

int closeFile(const char* pathname) {

}

int removeFile(const char* pathname) {

}
