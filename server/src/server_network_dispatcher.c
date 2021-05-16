#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../include/server_network_dispatcher.h"
#include "../include/server_packet_handler.h"

#define N 100
#define UNIX_PATH_MAX 108 
#define SOCKNAME "./mysock"

void *dispatch_connection() {

    pthread_exit(EXIT_SUCCESS);

    // struct sockaddr *psa;
    // strncpy(psa.sun_path, SOCKNAME, UNIX_PATH_MAX);
    // psa.sun_family = AF_UNIX;

    // struct sockaddr_un sa; 

    // strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
    // sa.sun_family = AF_UNIX;

    // int fd_sk, fd_c, fd_num = 0, fd;
    // char buf[N];
    // fd_set set, rdset;
    // int nread;
    
    // fd_sk = socket(AF_UNIX, SOCK_STREAM, 0);
    
    // bind(fd_sk, (struct sockaddr *) &sa, sizeof(sa)); //psa
    
    // listen(fd_sk,SOMAXCONN);
    
    // if (fd_sk > fd_num) {
    //     fd_num = fd_sk;
    // }

    // FD_ZERO(&set);
    // FD_SET(fd_sk,&set);
    
    // while (1) {
    //     /* preparo maschera per select */
    //     rdset = set; 
    //     if (select(fd_num + 1, &rdset, NULL, NULL, NULL) == -1) { /* gest errore */

    //     /* select OK */
    //     } else { 
    //         for (fd = 0; fd <= fd_num; fd++) {
    //             if (FD_ISSET(fd, &rdset)) {
    //                 /* sock connect pronto */
    //                 if (fd == fd_sk) { 
    //                     fd_c = accept(fd_sk, NULL, 0);
    //                     FD_SET(fd_c, &set);
    //                     if (fd_c > fd_num) {
    //                         fd_num = fd_c;
    //                     }
    //                 /* sock I/0 pronto */
    //                 } else {

    //                     pushPacket(fd);
    //                     // nread = read(fd, buf, N);
    //                     // /* EOF client finito */
    //                     // if (nread == 0) {
    //                     //     FD_CLR(fd, &set);
    //                     //     // fd_num = aggiorna(&set);
    //                     //     close(fd);

    //                     //     // AGGIUNGI PACKET LIST
    //                     // /* nread !=0 */
    //                     // } else {
    //                     //     printf("Server got : %s\n", buf);
    //                     //     write(fd, "Bye !", 5);
    //                     // }
    //                 }
    //             }
    //         }
    //     } 
    // }
}


