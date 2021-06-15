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
#include "../../core/include/utils.h"

#define UNIX_PATH_MAX 108 
#define BUFFER_RESPONSE_SIZE 100
#define PATH_SIZE 1024

static int SERVER_SOCKET = -1;

/**
 * Viene aperta una connessione AF_UNIX al socket file sockname. Se il server non accetta immediatamente la
 * richiesta di connessione, la connessione da parte del client viene ripetuta dopo ‘msec’ millisecondi e fino allo
 * scadere del tempo assoluto ‘abstime’ specificato come terzo argomento. Ritorna 0 in caso di successo, -1 in caso
 * di fallimento, errno viene settato opportunamente.
 **/
int openConnection(const char* sockname, int msec, const struct timespec abstime) {

    // Controllo se ho già effettuato una connessione al server
    if (SOCKET_PATH != NULL || SERVER_SOCKET != -1) {
        errno = EISCONN;
        return -1;
    }

    // Controllo se gli argomenti passati per parametro sono validi
    if(sockname == NULL || msec < 0) {
        errno = EINVAL;
        return -1;
    }

    struct sockaddr_un sa;
    memset(&sa, '0', sizeof(sa));
    strncpy(sa.sun_path, sockname, UNIX_PATH_MAX);
    sa.sun_family = AF_UNIX;

    if ((SERVER_SOCKET = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        errno = EINVAL;
        return -1;
    }

    int status;
    struct timespec currTime;
    clock_gettime(CLOCK_REALTIME, &currTime);
        
    while ((status = connect(SERVER_SOCKET, (struct sockaddr *) &sa, sizeof(sa))) == -1 && currTime.tv_sec < abstime.tv_sec) {
        printf("CLIENT: Nessuna connessione effettuata! Provo nuovamente a connettermi...\n");
        if (msec != 0) sleep(msec / 1000);
        clock_gettime(CLOCK_REALTIME, &currTime);
    }

    if (status == -1) {
        fprintf(stderr, "CLIENT: Impossibile stabilire una connessione con il server: Connection Timedout\n");
        errno = ETIMEDOUT;
        return -1;
    }

    if ((SOCKET_PATH = (char *) malloc(strlen(sockname) + 1)) == NULL) {
        perror("ERRORE: impossibile allocare memoria per SOCKET_PATH.");
        return -1;
    }
    strncpy(SOCKET_PATH, sockname, strlen(sockname) + 1);
    
    return 0;
}

/**
 * Chiude la connessione AF_UNIX associata al socket file sockname. Ritorna 0 in caso di successo, -1 in caso di
 * fallimento, errno viene settato opportunamente.
 **/
int closeConnection(const char* sockname) {
    if (SOCKET_PATH != NULL && strncmp(SOCKET_PATH, sockname, strlen(SOCKET_PATH) + 1) == 0)  {
        close(SERVER_SOCKET);
        return 0;
    }
    return -1;
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

    printf("CLIENT: Invio una open di \"%s\".\n", pathname);

    char absolutePath[PATH_SIZE];
    realpath(pathname, absolutePath);

    int byte;
    int id = OPEN_FILE;
    int pathLength = strlen(absolutePath) + 1;
    int ocreate, olock;

    switch (flags) {
        case O_CREATE:
            ocreate = 1;
            olock = 0;
            break;
        case O_LOCK:
            ocreate = 0;
            olock = 1;
            break;
        case O_CREATE | O_LOCK:
            ocreate = 1;
            olock = 1;
            break;
        default:
            ocreate = 0;
            olock = 0;
            break;
    }

    int payloadLength = sizeof(int) + pathLength + sizeof(int) + sizeof(int);

    DEBUG(("===================================================\n"));
    DEBUG(("HEADER\n"));
    DEBUG(("ID pacchetto: %d\n", id));
    DEBUG(("Dimensione Payload %d\n", payloadLength));
    DEBUG(("PAYLOAD\n"));
    DEBUG(("Lunghezza Path: %d\n", pathLength));
    DEBUG(("Path: %s\n", pathname));
    DEBUG(("O_CREATE: %d\n", ocreate));
    DEBUG(("O_LOCK: %d\n", olock));

    // Header
    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));

    // Payload
    char *payload = malloc(payloadLength);
    memcpy(payload, &pathLength, sizeof(int));
    memcpy(payload + sizeof(int), absolutePath, pathLength);
    memcpy(payload + sizeof(int) + pathLength, &ocreate, sizeof(int));
    memcpy(payload + sizeof(int) + pathLength + sizeof(int), &olock, sizeof(int));
    
    byte = write(SERVER_SOCKET, header, sizeof(int) * 2);
    DEBUG(("Invio Header: %d byte scritti\n", byte));

    byte = write(SERVER_SOCKET, payload, payloadLength);
    DEBUG(("Invio Payload: %d byte scritti\n", byte));

    // Response
    int response;
    byte = read(SERVER_SOCKET, &response, sizeof(int));

    DEBUG(("RESPONSE\n"));
    DEBUG(("Leggo Response: %d byte letti\n", byte));
    DEBUG(("Esito: %d\n", response));
    DEBUG(("===================================================\n"));

    free(payload);
    free(header);

    if (response != 1) {
        errno = ENOENT;
    }
    
    switch (response) {
        case 1:
            printf("SERVER: Apertura del File eseguita con successo!\n");
            break;
        case 0:
            printf("SERVER: Impossibile aprire il File richiesto.\n");
            break;
        case -1:
            printf("SERVER: Impossibile eseguire open multiple sullo stesso file!\n");
            break;
        case -2:
            printf("SERVER: Impossibile eseguire la open sul file richiesto perché è in stato di Locked\n"); 
            break;
        case -3:
            printf("SERVER: Impossibile eseguire la open sul file con il flag di Lock a 1 perché in questo momento è aperto da altri utenti\n");
            break;
        default:
            break;
    }

    return response == 1 ? 0 : -1;
}

/**
 * Legge tutto il contenuto del file dal server (se esiste) ritornando un puntatore ad un'area allocata sullo heap nel
 * parametro ‘buf’, mentre ‘size’ conterrà la dimensione del buffer dati (ossia la dimensione in bytes del file letto). In
 * caso di errore, ‘buf‘e ‘size’ non sono validi. Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene
 * settato opportunamente.
 **/
int readFile(const char* pathname, void** buf, size_t* size) {
    
    printf("CLIENT: Invio una read di \"%s\".\n", pathname);

    char absolutePath[PATH_SIZE];
    realpath(pathname, absolutePath);

    int byte = 0;
    int id = READ_FILE;
    int pathLength = strlen(absolutePath) + 1;
    int payloadLength = pathLength;

    DEBUG(("===================================================\n"));
    DEBUG(("HEADER\n"));
    DEBUG(("ID pacchetto: %d\n", id));
    DEBUG(("Dimensione Payload %d\n", payloadLength));
    DEBUG(("PAYLOAD\n"));
    DEBUG(("Lunghezza Path: %d\n", pathLength));
    DEBUG(("Path: %s\n", pathname));

    // Header
    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));    

    // Payload
    char *payload = malloc(payloadLength);
    memcpy(payload, absolutePath, pathLength);

    byte = write(SERVER_SOCKET, header, sizeof(int) * 2);
    DEBUG(("Invio Header: %d byte scritti\n", byte));

    byte = write(SERVER_SOCKET, payload, payloadLength);
    DEBUG(("Invio Payload: %d byte scritti\n", byte));

    // Response Header
    char *headerResponse = malloc(sizeof(int));
    byte = read(SERVER_SOCKET, headerResponse,  sizeof(int));

    DEBUG(("RESPONSE\n"));
    DEBUG(("Leggo Header Response: %d byte letti\n", byte));

    int packetSize = *((int *) headerResponse);

    // Se il packetSize è uguale a 0, allora il client non ha fatto la open
    if (packetSize == 0) {
        free(headerResponse);
        free(payload);
        free(header);

        // Setto errno
        errno = ENOENT;

        DEBUG(("Esito: 0\n"));
        DEBUG(("===================================================\n"));
        printf("SERVER: Devi prima richiedere di aprire il File!\n");

        // Restituisco errore
        return -1;
    }
    // Response Payload
    char *payloadResponse = (char *) malloc(sizeof(char) * packetSize);
    byte = read(SERVER_SOCKET, payloadResponse, packetSize);

    DEBUG(("Leggo Payload Response: %d byte letti\n", byte));
    DEBUG(("Contenuto Payload Response: %s\n", payloadResponse));
    DEBUG(("Esito: 1\n"));
    DEBUG(("===================================================\n"));

    printf("SERVER: Contenuto del messaggio: \"%s\"\n", payloadResponse);

    if (buf != NULL || size != NULL) {
        *size = packetSize;
        *buf = payloadResponse;
    } else {
        free(payloadResponse);
    }

    free(headerResponse);
    free(payload);
    free(header);

    return 0;
}

/**
 * Richiede al server la lettura di ‘N’ files qualsiasi da memorizzare nella directory ‘dirname’ lato client. Se il server
 * ha meno di ‘N’ file disponibili, li invia tutti. Se N<=0 la richiesta al server è quella di leggere tutti i file
 * memorizzati al suo interno. Ritorna un valore maggiore o uguale a 0 in caso di successo (cioè ritorna il n. di file
 * effettivamente letti), -1 in caso di fallimento, errno viene settato opportunamente.
 **/
int readNFiles(int N, const char* dirname) {

    int id = READ_N_FILES;
    int payloadLength = sizeof(int);

    // Header
    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // Payload
    char *payload = malloc(payloadLength);
    memcpy(payload, &N, sizeof(int));

    write(SERVER_SOCKET, header, sizeof(int) * 2);
    write(SERVER_SOCKET, payload, payloadLength);        

    // Response Header
    char *headerResponse = malloc(sizeof(int));

    read(SERVER_SOCKET, headerResponse,  sizeof(int));
    int packetSize = *((int *) headerResponse);

    // Response Payload
    char *payloadResponse = (char *) malloc(sizeof(char) * packetSize);

    read(SERVER_SOCKET, payloadResponse, packetSize);    

    int filesAmount =  *((int *) payloadResponse);

    printf("%d\n", filesAmount);

    char *currentPosition = payloadResponse + sizeof(int);
    for (int i = 0; i < filesAmount; i++) {

        int pathLength = *((int *) currentPosition);
        currentPosition += sizeof(int); 

        char *path = malloc(sizeof(char) * pathLength);
        strncpy(path, currentPosition, pathLength);
        currentPosition += pathLength;

        int contentLength = *((int *) currentPosition);
        currentPosition += sizeof(int);
        
        char *content = malloc(sizeof(char) * contentLength);
        strncpy(content, currentPosition, contentLength);
        currentPosition += contentLength;

        printf("%d\n", pathLength);
        printf("%d\n", contentLength);

        printf("File Path: %s\n", path);
        printf("File Content: %s\n", content);

        free(content);
        free(path);
    }
    
    free(payloadResponse);
    free(headerResponse);
    free(payload);
    free(header);

    return 0;
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
        return errno;
    }

    printf("CLIENT: Invio una write di \"%s\".\n", pathname);
    
    char absolutePath[PATH_SIZE];
    realpath(pathname, absolutePath);

    fseek(file, 0, SEEK_END);
    int textLength = ftell(file);
    fseek(file, 0, SEEK_SET);
    rewind(file);

    char *content = (char *) malloc(sizeof(char) * textLength + 1);
    int cont = 0;

    while (cont < textLength) {
        cont += fread(content, sizeof(char), textLength, file);
    }
    content[textLength] = '\0';

    int byte;
    int id = WRITE_FILE;
    int pathLength = strlen(absolutePath) + 1;
    int contentLength = strlen(content) + 1;
    int payloadLength = sizeof(int) + pathLength + sizeof(int) + contentLength;

    DEBUG(("===================================================\n"));
    DEBUG(("HEADER\n"));
    DEBUG(("ID pacchetto: %d\n", id));
    DEBUG(("Dimensione Payload %d\n", payloadLength));
    DEBUG(("PAYLOAD\n"));
    DEBUG(("Lunghezza Path: %d\n", pathLength));
    DEBUG(("Path: %s\n", absolutePath));
    DEBUG(("Lunghezza Content: %d\n", contentLength));
    DEBUG(("Content: %s\n", content));

    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // Payload
    char *payload = malloc(payloadLength);
    char *currentPosition = payload;

    memcpy(currentPosition, &pathLength, sizeof(int));
    currentPosition += sizeof(int);
    memcpy(currentPosition, absolutePath, pathLength);
    currentPosition += pathLength;
    memcpy(currentPosition, &contentLength, sizeof(int));
    currentPosition += sizeof(int);
    memcpy(currentPosition, content, contentLength);

    byte = write(SERVER_SOCKET, header, sizeof(int) * 2);
    DEBUG(("Invio Header: %d byte scritti\n", byte));
    byte = write(SERVER_SOCKET, payload, payloadLength);
    DEBUG(("Invio Payload: %d byte scritti\n", byte));
    
    // Response
    int response;
    byte = read(SERVER_SOCKET, &response, sizeof(int));

    DEBUG(("RESPONSE\n"));
    DEBUG(("Leggo Response: %d byte letti\n", byte));
    DEBUG(("Esito: %d\n", response));
    DEBUG(("===================================================\n"));
    
    free(content);
    free(header);
    free(payload);

    if (fclose(file) != 0) {
        perror("ERRORE: Chiusura del File");
        return -1;
    }

    if (response == 0) {
        errno = ENOENT;

        printf("SERVER: Devi prima richiedere di aprire il File!\n");
        return -1;
    }

    printf("SERVER: Richiesta di write eseguita con successo.\n");

    return 0;
}

/**
 * Richiesta di scrivere in append al file ‘pathname‘ i ‘size‘ bytes contenuti nel buffer ‘buf’. L’operazione di append
 * nel file è garantita essere atomica dal file server. Se ‘dirname’ è diverso da NULL, il file eventualmente spedito
 * dal server perchè espulso dalla cache per far posto ai nuovi dati di ‘pathname’ dovrà essere scritto in ‘dirname’;
 * Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene settato opportunamente.
 **/
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname) {

    int byte;
    int id = APPEND_TO_FILE;
    int pathLength = strlen(pathname) + 1;
    int contentLength = strlen(buf) + 1;
    int payloadLength = sizeof(int) + pathLength + sizeof(int) + contentLength;

    DEBUG(("===================================================\n"));
    DEBUG(("HEADER\n"));
    DEBUG(("ID pacchetto: %d\n", id));
    DEBUG(("Dimensione Payload %d\n", payloadLength));
    DEBUG(("PAYLOAD\n"));
    DEBUG(("Lunghezza Path: %d\n", pathLength));
    DEBUG(("Path: %s\n", pathname));
    DEBUG(("Lunghezza Content: %d\n", contentLength));
    DEBUG(("Content: %s\n", (char *) buf));

    // Header
    char *header = (char *) malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // Payload
    char *payload = malloc(payloadLength);
    char *currentPosition = payload;

    memcpy(currentPosition, &pathLength, sizeof(int));
    currentPosition += sizeof(int);
    memcpy(currentPosition, pathname, pathLength);
    currentPosition += pathLength;
    memcpy(currentPosition, &contentLength, pathLength);
    currentPosition += sizeof(int);
    memcpy(currentPosition, buf, contentLength);

    byte = write(SERVER_SOCKET, header, sizeof(int) * 2);
    DEBUG(("Invio Header: %d byte scritti\n", byte));
    byte = write(SERVER_SOCKET, payload, payloadLength);
    DEBUG(("Invio Payload: %d byte scritti\n", byte));

    // Response
    int response;
    byte = read(SERVER_SOCKET, &response, sizeof(int));
    
    DEBUG(("RESPONSE\n"));
    DEBUG(("Leggo Response: %d byte letti\n", byte));
    DEBUG(("Esito: %d\n", response));
    DEBUG(("===================================================\n"));

    free(payload);
    free(header);

    if (response == 0) {
        errno = ENOENT;
        printf("SERVER: Devi prima richiedere di aprire il File!\n");
    }

    return response == 1 ? 0 : -1;
}

int lockFile(const char* pathname);

int lockFile(const char* pathname);

/**
 * Richiesta di chiusura del file puntato da ‘pathname’. Eventuali operazioni sul file dopo la closeFile falliscono.
 * Ritorna 0 in caso di successo, -1 in caso di fallimento, errno viene settato opportunamente.
 **/
int closeFile(const char* pathname) {

    printf("CLIENT: Invio una close di \"%s\".\n", pathname);
    
    char absolutePath[STRING_SIZE];
    realpath(pathname, absolutePath);    

    int byte;
    int id = CLOSE_FILE;
    int pathLength = strlen(absolutePath) + 1;
    int payloadLength = pathLength;

    DEBUG(("===================================================\n"));
    DEBUG(("HEADER\n"));
    DEBUG(("ID pacchetto: %d\n", id));
    DEBUG(("Dimensione Payload %d\n", payloadLength));
    DEBUG(("PAYLOAD\n"));
    DEBUG(("Lunghezza Path: %d\n", pathLength));
    DEBUG(("Path: %s\n", pathname));

    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // payload
    char *payload = malloc(payloadLength);
    memcpy(payload, absolutePath, pathLength);

    byte = write(SERVER_SOCKET, header, sizeof(int) * 2);
    DEBUG(("Invio Header: %d byte scritti\n", byte));
    byte = write(SERVER_SOCKET, payload, payloadLength);
    DEBUG(("Invio Payload: %d byte scritti\n", byte));
    
    // Response
    int response;
    byte = read(SERVER_SOCKET, &response, sizeof(int));
    
    DEBUG(("RESPONSE\n"));
    DEBUG(("Leggo Response: %d byte letti\n", byte));
    DEBUG(("Esito: %d\n", response));
    DEBUG(("===================================================\n"));
    
    free(payload);
    free(header);

    if (response == 0) {
        errno = ENOENT;
        printf("SERVER: Devi prima richiedere di aprire il File!\n");
        return -1;
    }

    printf("SERVER: Close eseguita con successo\n");

    return 0;
}

/**
 * Rimuove il file cancellandolo dal file storage server. L’operazione fallisce se il file non è in stato locked, o è in
 * stato locked da parte di un processo client diverso da chi effettua la removeFile.
 **/
int removeFile(const char* pathname) {

    printf("CLIENT: Invio una remove di \"%s\".\n", pathname);

    char absolutePath[STRING_SIZE];
    realpath(pathname, absolutePath);    

    int byte;
    int id = REMOVE_FILE;
    int pathLength = strlen(absolutePath) + 1;
    int payloadLength = pathLength;

    DEBUG(("===================================================\n"));
    DEBUG(("HEADER\n"));
    DEBUG(("ID pacchetto: %d\n", id));
    DEBUG(("Dimensione Payload %d\n", payloadLength));
    DEBUG(("PAYLOAD\n"));
    DEBUG(("Lunghezza Path: %d\n", pathLength));
    DEBUG(("Path: %s\n", pathname));

    // Header
    char *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + sizeof(int), &payloadLength, sizeof(int));        

    // payload
    char *payload = malloc(payloadLength);
    memcpy(payload, &pathLength, sizeof(int));
    memcpy(payload + sizeof(int), absolutePath, pathLength);

    byte = write(SERVER_SOCKET, header, sizeof(int) * 2);
    DEBUG(("Invio Header: %d byte scritti\n", byte));
    byte = write(SERVER_SOCKET, payload, payloadLength);
    DEBUG(("Invio Payload: %d byte scritti\n", byte));

    // Response
    int response;
    byte = read(SERVER_SOCKET, &response, sizeof(int));

    DEBUG(("RESPONSE\n"));
    DEBUG(("Leggo Response: %d byte letti\n", byte));
    DEBUG(("Esito: %d\n", response));
    DEBUG(("===================================================\n"));

    free(payload);
    free(header);

    if (response == 0) {
        errno = ENOENT;
        printf("SERVER: Devi prima richiedere di aprire il File!\n");
        return -1;
    }

    
    printf("SERVER: Remove eseguita con successo\n");

    return 0;
}


