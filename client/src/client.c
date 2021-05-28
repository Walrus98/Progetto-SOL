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

    int id = 3;
    char message[5] = "ciao"; // c i a o \0
    int length = strlen(message) + 1;


    // header
    write(fd_skt, &id, sizeof(int));
    write(fd_skt, &length, sizeof(int));
    
    // payload
    write(fd_skt, message, length);


    sleep(5);
    read(fd_skt, buf, N);
    printf("Client got : %s\n", buf);
    close(fd_skt);

    return 0;
}



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