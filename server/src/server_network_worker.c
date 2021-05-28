#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>

#include "../include/server_network_worker.h"
#include "../include/server_network_handler.h"

#define CHECK_CONNECTION(buffer, pipeTask, fd, size)     \
    nread = readn(fd, buffer, size);                     \
    if (nread == 0 || nread == -1)                       \
    {                                                    \
        int connectedClients = -1;                       \
        write(pipeTask, &connectedClients, sizeof(int)); \
        close(fd);                                       \
        continue;                                        \
    }

/* Read "n" bytes from a descriptor */

/** Evita letture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la lettura da fd leggo EOF
 *   \retval size se termina con successo
 */
static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char *)buf;
    while (left > 0) {
        if ((r = read((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        // EOF
        if (r == 0) {
            return 0; 
        }        
        left -= r;
        bufptr += r;
    }

    return size;
}

void *handle_connection(void *pipeHandleClient) {

    int *pipeTask = (int *) pipeHandleClient;

    void *header = malloc(sizeof(int));
    void *payload = NULL;
    int nread;

    while (CONNECTION == 1) {
        int fileDescriptor = popPacket();
        
        if (fileDescriptor == -1) {
            break;
        }

        // leggo l'header
        CHECK_CONNECTION(header, pipeTask[1], fileDescriptor, sizeof(int));
        int id = *((int *) header);

        CHECK_CONNECTION(header, pipeTask[1], fileDescriptor, sizeof(int));
        int length = *((int *) header);
        payload = malloc(length);
        
        // leggo il paylod
        CHECK_CONNECTION(payload, pipeTask[1], fileDescriptor, length);
        char *testo = (char *) payload;

        printf("ID -> %d\n", id);
        printf("Length -> %d\n", length);
        printf("Message -> %s\n", testo);

        write(fileDescriptor, "Bye !", 5);

        // Attraverso la pipe mando indietro il fd al Thread Dispatcher
        write(pipeTask[1], &fileDescriptor, sizeof(int));
              
    }

    close(pipeTask[1]);
    printf("VADO A SUICIDARMI\n");
    
    return NULL;
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



// aaaaaaaaaaa

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
        
        /**
         * Ricevo il pacchetto con una struct
         */

        // int size = 0;

        // int nread = read(fileDescriptor, buffer, sizeof(int));
        // if (nread != 0) {
        //     size = *((int *) buffer);
        // }
        
        // nread = read(fileDescriptor, buffer, size);
        // if (nread != 0) {
        //     Packet packet = *((Packet *) buffer);
        //     id = packet.id;
        //     length = packet.length;
        //     // testo = packet.message;
        //     printf("test %s\n", packet.message);
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



// ssize_t readn(int fd, void *ptr, size_t n) {

//     size_t bytesRead = 0;
//     do {
//         ptr += bytesRead;
//         bytesRead = read(fd, ptr, n);
//         n -= bytesRead;

//         printf("bytesRead %ld\n", bytesRead);

//     } while (n > 0 && bytesRead > 0);

//     printf("finito!\n");

//     return bytesRead <= 0 ? bytesRead : 1;
// }
