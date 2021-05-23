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
#include "../include/server_packet_handler.h"

void *handle_connection(void *pfd) {

    // char msg[4];

    // int *test = (int *) pfd;

    // close(test[0]);
    
    // close(*(((int *) pfd) + 0));

    // int l = write(test[1], "yolo", 4);

    // int l = write(*(((int *) pfd) + 1), "yolo", 100);

    // close(test[1]);

    // close(*(((int *) pfd) + 1));

    // ============================= //

    char buffer[100];

    while (CONNECTION) {
        int fileDescriptor = popPacket();
        
        if (fileDescriptor == -1) {
            break;
        }
        
        int nread = read(fileDescriptor, buffer, N);
        if (nread == 0) {
            close(fileDescriptor);
        } else {
            printf("Server got : %s\n", buffer);
            write(fileDescriptor, "Bye !", 5);

            // con pipe mando indietro il fd
        }            
    }

    printf("VADO A SUICIDARMI\n");
    pthread_exit(EXIT_SUCCESS);

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
