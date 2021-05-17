#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
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

static int aggiorna(fd_set set, int fd_max) {
    for (int i = fd_max - 1; i >= 0; i--) {
        if (FD_ISSET(i, &set)) {
            return i;
        }
    }

    return -1;
}

void *dispatch_connection() {
    
    // Creo la struct del socket e assegno proprietà
    struct sockaddr_un sa; 
    strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
    sa.sun_family = AF_UNIX;    

    // Creo il socket
    int fd_sk = socket(AF_UNIX, SOCK_STREAM, 0);
    
    // Collego il socket alla struct del socket con le proprietà
    bind(fd_sk, (struct sockaddr *) &sa, sizeof(sa)); 
    
    // Prepara il socket ad accettere le connessioni, con il numero massimo di client che possono stare in coda ad aspettare che il server gli risponda 
    listen(fd_sk, SOMAXCONN);
    
    // Dichiaro i due set
    fd_set set;
    fd_set rdset;

    // Resetta il set
    FD_ZERO(&set);

    // Registro nel set il server socket
    FD_SET(fd_sk, &set);

    int fd_num = fd_sk;
    while (1) {
        // Copio in rdset i valori contenuti in set
        rdset = set; 
        // Prendo tutti gli elementi che sono pronti ad eseguire operazioni sul server o client
        if (select(fd_num + 1, &rdset, NULL, NULL, NULL) == -1) {
            fprintf(stderr, "ERRORE: Impossibile eseguire la select nel Thread Dispatcher");
            exit(EXIT_FAILURE);
        } else { 
            // Scorro tutti i possibili file descriptor registrati
            for (int fd = 0; fd <= fd_num; fd++) {
                // Controllo se il file descriptor che sto iterando è registrato dalla select in set
                if (FD_ISSET(fd, &rdset)) {
                    // Se il file descriptor che sto iterando è il serverSocket
                    if (fd == fd_sk) { 
                        // Allora significa che devo accettare una nuova connessione e registrare anche essa all'interno del set
                        int fd_c = accept(fd_sk, NULL, 0);
                        // Registro il nuovo client sul set
                        FD_SET(fd_c, &set);

                        // Se il file descriptor ha un ID che è maggiore di fd_num, dove fd_num corrisponde all'id massimo dei file descriptor
                        // registrati dalla select, allora aggiorno il valore di fd_num con il nuovo valore massimo
                        if (fd_c > fd_num) {
                            fd_num = fd_c;
                        }
                    // Altrimenti significa che un client ha inviato un pacchetto al server  
                    } else {
                        // Creo un Buffer per leggere il messaggio inviato dal client
                        char buf[N];
                        // Leggo il messaggio inviato dal client
                        int nread = read(fd, buf, N);
                        
                        // Se nread = 0, significa che il client ha terminato la conenssione
                        if (nread == 0) {
                            FD_CLR(fd, &set);
                            fd_num = aggiorna(set, fd_num);
                            close(fd);
                        } else {
                            // Rispondo al client
                            printf("Server got : %s\n", buf);
                            write(fd, "Bye !", 5);
                        }
                    }
                }
            }
        } 
    }

    pthread_exit(EXIT_SUCCESS);

    // int fd_sk, fd_c, fd_num = 0, fd;
    // char buf[N];
    // fd_set set, rdset;
    // int nread;

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


