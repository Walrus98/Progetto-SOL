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
    write(fd_skt,"Hello!", 10);
    read(fd_skt, buf, N);
    printf("Client got : %s\n", buf);
    close(fd_skt);
    exit(EXIT_SUCCESS);

    return 0;
}