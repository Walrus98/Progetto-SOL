#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../include/server_network_worker.h"
#include "../include/server_network_handler.h"

typedef struct Packet {
    int id;
    int length;
    char *message;
} Packet;

void *handle_connection(void *pipeHandleClient) {

    int *pipeTask = (int *) pipeHandleClient;

    char buffer[100];

    while (CONNECTION == 1) {
        int fileDescriptor = popPacket();
        
        if (fileDescriptor == -1) {
            break;
        }

        int id = 0;
        int length = 0;
        char *testo = NULL;
        
        // int nread = read(fileDescriptor, buffer, N);
        
        /**
         * Ricevo il pacchetto con 3 read
         */

        // int nread = read(fileDescriptor, buffer, sizeof(int));
        // if (nread != 0) {
        //     id = *((int *) buffer);
        // }

        // nread = read(fileDescriptor, buffer, sizeof(int));
        // if (nread != 0) {
        //     length = *((int *) buffer);
        // }

        // nread = read(fileDescriptor, buffer, length);
        // if (nread != 0) {
        //     testo = buffer;
        // }

        // Pacchetto inviato con il buffer

        // int size = 0;

        // int nread = read(fileDescriptor, buffer, sizeof(int));
        // if (nread != 0) {
        //     size = *((int *) buffer);
        // }
        
        // nread = read(fileDescriptor, buffer, size);
        // if (nread != 0) {
        //     id = *((int *) buffer);
        //     length = *((int *) buffer + 4);
        //     testo = ((char *) buffer + 8);
        // }

        
        /**
         * Ricevo il pacchetto con una struct
         */

        int size = 0;

        int nread = read(fileDescriptor, buffer, sizeof(int));
        if (nread != 0) {
            size = *((int *) buffer);
        }
        
        nread = read(fileDescriptor, buffer, size);
        if (nread != 0) {
            Packet packet = *((Packet *) buffer);
            id = packet.id;
            length = packet.length;
            testo = packet.message;
        }

        if (nread == 0) {

            int connectedClients = -1;

            write(pipeTask[1], &connectedClients, sizeof(int));

            close(fileDescriptor);            
        } else {
            
            printf("size -> %d\n", size);
            printf("ID -> %d\n", id);
            printf("Length -> %d\n", length);
            printf("Message -> %s\n", testo);

            


            // printf("Server got : %s\n", buffer);
            write(fileDescriptor, "Bye !", 5);

            // con pipe mando indietro il fd

            write(pipeTask[1], &fileDescriptor, sizeof(int));
        }            
    }

    close(pipeTask[1]);
    printf("VADO A SUICIDARMI\n");
    
    return NULL;

    // // Creo un Buffer per leggere il messaggio inviato dal client
    // char buf[N];
    // // Leggo il messaggio inviato dal client
    // int nread = read(fd, buf, N);
    
    // // Se nread = 0, significa che il client ha terminato la conenssione
    // if (nread == 0) {
    //     FD_CLR(fd, &set);
    //     fd_num = aggiorna(set, fd_num);
    //     close(fd);
    // // Altrimenti rispondo al client
    // } else {
    //     printf("Server got : %s\n", buf);
    //     write(fd, "Bye !", 5);
    // }


}

// /* Read "n" bytes from a descriptor */
// ssize_t readn(int fd, void *ptr, size_t n) {
//     size_t nleft;
//     ssize_t nread;

//     nleft = n;
//     while (nleft > 0) {
//         if ((nread = read(fd, ptr, nleft)) < 0) {
//             if (nleft == n)
//                 return -1; /* error, return -1 */
//             else
//                 break; /* error, return amount read so far */
//         }
//         else if (nread == 0)
//             break; /* EOF */
//         nleft -= nread;
//         ptr += nread;
//     }
//     return (n - nleft); /* return >= 0 */
// }

// /* Write "n" bytes to a descriptor */
// ssize_t writen(int fd, void *ptr, size_t n) {
//     size_t nleft;
//     ssize_t nwritten;

//     nleft = n;
//     while (nleft > 0) {
//         if ((nwritten = write(fd, ptr, nleft)) < 0) {
//             if (nleft == n)
//                 return -1; /* error, return -1 */
//             else
//                 break; /* error, return amount written so far */
//         }
//         else if (nwritten == 0)
//             break;
//         nleft -= nwritten;
//         ptr += nwritten;
//     }
//     return (n - nleft); /* return >= 0 */
// }
