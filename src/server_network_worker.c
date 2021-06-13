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


#include "../include/server_network_worker.h"
#include "../include/server_network_handler.h"
#include "../include/server_packet_handler.h"

#define READ_PACKET(buffer, pipeTask, fd, size)     \
    nread = readn(fd, buffer, size);                     \
    if (nread == 0 || nread == -1)                       \
    {                                                    \
        int connectedClients = -1;                       \
        write(pipeTask, &connectedClients, sizeof(int)); \
        handleDisconnect(fd);                            \
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

    char *packetHeader = malloc(sizeof(int) * 2);
    char *packetPayload = NULL;
    int nread;

    while (CONNECTION == 1) {
        int fileDescriptor = popPacket();
        
        if (fileDescriptor == -1) {
            break;
        }

        // leggo l'header del pacchetto
        READ_PACKET(packetHeader, pipeTask[1], fileDescriptor, sizeof(int) * 2);
        int packetID = *((int *) packetHeader);
        int packetSize = *((int *) packetHeader + 1);

        // Alloco la dimensione del buffer payload
        packetPayload = malloc(packetSize);

        // leggo il paylod del pacchetto
        READ_PACKET(packetPayload, pipeTask[1], fileDescriptor, packetSize);
        
        // Gestisco il pacchetto ricevuto
        handlePacket(packetID, packetSize, packetPayload, fileDescriptor);

        // Attraverso la pipe mando indietro il fd al Thread Dispatcher
        write(pipeTask[1], &fileDescriptor, sizeof(int));     
  
        // Libero il payload allocato precedentemente
        free(packetPayload);
    }

    close(pipeTask[1]);
    free(packetHeader);

    printf("VADO A SUICIDARMI\n");
    
    return NULL;
}



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
