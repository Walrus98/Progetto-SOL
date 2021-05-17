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

void *handle_connection() {

    int fileDescriptor = popPacket();
    
    char buf[100];
    int nread = read(fileDescriptor, buf, 10);
    printf("Server got : %s\n", buf);
    write(fileDescriptor, "BONE BONE !", 12);

    pthread_exit(EXIT_SUCCESS);

    // int fd = popPacket();

    // nread = read(fd, buf, N);
    // /* EOF client finito */
    // if (nread == 0) {
    //     FD_CLR(fd, &set); // pipe
    //     // fd_num = aggiorna(&set);
    //     close(fd);

    //     // AGGIUNGI PACKET LIST
    // /* nread !=0 */
    // } else {
    //     printf("Server got : %s\n", buf);
    //     write(fd, "Bye !", 5);
    // }


}