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
static FILE *file;

int open_connection(const char* sockname, int msec, const struct timespec abstime) {
    
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

int close_connection(const char* sockname) {
    close(SERVER_SOCKET);
}

int openFile(const char* pathname, int flags) {

    file = NULL;
    if ((file = fopen(pathname, "r")) == NULL) {
        perror("ERRORE: apertura del file");
        exit(errno);
    }

    printf("Invio una open di %s\n", pathname);

    int id = OPEN_FILE;
    int pathLength = strlen(pathname) + 1;
    int ocreate = 1;
    int olock = 0;
    int payloadLength = sizeof(int) + pathLength + sizeof(int) +  sizeof(int);

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
}

int readFile(const char* pathname, void** buf, size_t* size) {

}

int readNFiles(int N, const char* dirname) {

}

int writeFile(const char* pathname, const char* dirname) {

    if (file == NULL) {
        return -1;
    }

    char content[1024];
    fgets(content, 1024, file);

    printf("Invio una write di %s\n", pathname);

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
    memcpy(currentPosition, content, pathLength);

    write(SERVER_SOCKET, header, sizeof(int) * 2);
    write(SERVER_SOCKET, payload, payloadLength);
    
    // Response
    char *response = malloc(sizeof(char) * 100);

    read(SERVER_SOCKET, response, BUFFER_RESPONSE_SIZE);
    printf("Messaggio ricevuto: %s\n\n", response);
    
    free(header);
    free(payload);
    free(response);
}

int closeFile(const char* pathname) {

}

int removeFile(const char* pathname) {

}
