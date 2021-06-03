#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define UNIX_PATH_MAX 108 
#define SOCKNAME "../../mysock"
#define N 100

int main(void) {

    int fd_skt, fd_c;
    char buf[N];
    struct sockaddr_un sa;
    strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
    sa.sun_family = AF_UNIX;

    fd_skt = socket(AF_UNIX,SOCK_STREAM,0);

    while (connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        if (errno == ENOENT) {
            printf("NONE!");
            sleep(1); /* sock non esiste */
        } else
            exit(EXIT_FAILURE);
    }

    int id = 0;
    char nameFile[5] = "ciao"; // c i a o \0
    int nameLength = strlen(nameFile) + 1;

    // char contentFile[13] = "hello world!";
    // int contentLength = strlen(contentFile) + 1; 

    int ocreate = 0;
    int olock = 0;

    int payloadLength = sizeof(int) + nameLength + sizeof(int) +  sizeof(int);

    void *header = malloc(sizeof(int) * 2);
    memcpy(header, &id, sizeof(int));
    memcpy(header + 4, &payloadLength, sizeof(int));
    
    // payload

    void *payload = malloc(payloadLength);
    memcpy(payload, &nameLength, sizeof(int));
    memcpy(payload + 4, nameFile, nameLength);
    memcpy(payload + 4 + nameLength, &ocreate, sizeof(int));
    memcpy(payload + 4 + nameLength + 4, &olock, sizeof(int));
    // memcpy(payload + 4 + nameLength, &contentLength, sizeof(int));
    // memcpy(payload + 4 + nameLength + 4, contentFile, contentLength);



    for (int i = 0; i < 2; i++) {
        write(fd_skt, header, sizeof(int) * 2);
        write(fd_skt, payload, payloadLength);
        read(fd_skt, buf, N);
        printf("Client got : %s\n", buf);
    }



    // sleep(5);

    close(fd_skt);

    return 0;
}

    // header

    // write(fd_skt, &id, sizeof(int));
    // write(fd_skt, &length, sizeof(int));

    /**
     * Mando il pacchetto con 3 write
     */

    // write(fd_skt, &id, sizeof(int));
    // write(fd_skt, &length, sizeof(int));
    // write(fd_skt, message, length);

    /**
     * Creo un buffer, invio prima la dimensione e poi invio il pacchetto
     */

    // id + size; 
    // faccio malloc size

    // int size = strlen(message) + 1 + sizeof(int) + sizeof(int);
    // void *buffer = malloc(sizeof(size));

    // memcpy(buffer, &id, sizeof(int));
    // memcpy(buffer + 4, &length, sizeof(int));
    // memcpy(buffer + 8, message, strlen(message) + 1);

    // write(fd_skt, &size, sizeof(int));
    // write(fd_skt, buffer, size);


    // ==============

    /**
     * Mando il pacchetto con una struct
     */

    // int size = strlen(message) + 1 + sizeof(int) + sizeof(int);

    // Packet packet;
    // packet.id = id;
    // packet.length = length;

    // packet.message = (char *) malloc(sizeof(char) * length);
    // // packet.message = testo;

    // write(fd_skt, &size, sizeof(int));
    // write(fd_skt, (void *) &packet, size);

    // write(fd_skt,"Hello!", 10);