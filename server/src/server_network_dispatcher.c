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
#include "../include/server_network_handler.h"

#define UNIX_PATH_MAX 108 
#define SOCKNAME "./mysock"


static int aggiorna(fd_set set, int fd_max) {
    for (int i = fd_max; i >= 0; i--) {
        if (FD_ISSET(i, &set)) {
            return i;
        }
    }

    return -1;
}

static int connectedClients = 0;
static int ACCEPT_CONNECTION = 1;

void *dispatch_connection(void *dispatcherArgument) {

    int pipeHandleConnection[2];
    int pipeHandleClient[2];
    DispatcherArg *argument = (DispatcherArg *) dispatcherArgument;

    pipeHandleConnection[0] = argument->pipeHandleConnection[0];
    pipeHandleConnection[1] = argument->pipeHandleConnection[1];
    pipeHandleClient[0] = argument->pipeHandleClient[0];
    pipeHandleClient[1] = argument->pipeHandleClient[1];

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

    // Registro nel set la pipe per terminare la connessione
    FD_SET(pipeHandleConnection[0], &set);
    
    // Registro nel set la pipe per inserire nuovamente i file descriptor inviati precedentemente ai thread worker 
    FD_SET(pipeHandleClient[0], &set);

    int fd_num;
    if (fd_sk >= pipeHandleConnection[0] && fd_sk >= pipeHandleClient[0]) {
        fd_num = fd_sk;
    } else if (pipeHandleConnection[0] >= pipeHandleClient[0] && pipeHandleConnection[0] >= fd_sk) {
        fd_num = pipeHandleConnection[0];
    } else {
        fd_num = pipeHandleClient[0];
    }

    while (CONNECTION == 1) {
        // Copio in rdset i valori contenuti in set
        rdset = set;

        if (connectedClients == 0 && ACCEPT_CONNECTION == 0) {            
            CONNECTION = 0;
            broadcast();
            break;
        }

        // Prendo dal rdset tutti gli elementi che sono pronti ad eseguire operazioni di lettura
        if (select(fd_num + 1, &rdset, NULL, NULL, NULL) == -1) {
            fprintf(stderr, "ERRORE: Impossibile eseguire la select nel Thread Dispatcher");
            exit(EXIT_FAILURE);
        } else {             
            // Scorro tutti i possibili file descriptor registrati
            for (int fd = 0; fd <= fd_num; fd++) {
                // Controllo se il file descriptor che sto iterando è registrato dalla select in set
                if (FD_ISSET(fd, &rdset)) {
                    // Se il file descriptor che sto iterando è il serverSocket
                    if (fd == fd_sk && ACCEPT_CONNECTION == 1) { 
                        // Allora significa che devo accettare una nuova connessione e registrare anche essa all'interno del set
                        int fd_c = accept(fd_sk, NULL, 0);
                        // Registro il nuovo client sul set
                        FD_SET(fd_c, &set);

                        // Se il file descriptor ha un ID che è maggiore di fd_num, dove fd_num corrisponde all'id massimo dei file descriptor
                        // registrati dalla select, allora aggiorno il valore di fd_num con il nuovo valore massimo
                        if (fd_c > fd_num) {
                            fd_num = fd_c;
                        }

                        connectedClients++;

                    // Se il file descriptor che sto iterando è la pipe di terminazione della connessione
                    } else if (fd == pipeHandleConnection[0]) {

                        char message[10];

                        if (read(pipeHandleConnection[0], &message, 10) == -1) {
                            perror("ERRORE PIPE\n");
                        }

                        printf("HO LETTO %s\n", message);

                        if (strncmp(message, "stop", 10) == 0) {
                            ACCEPT_CONNECTION = 0;

                            FD_CLR(pipeHandleConnection[0], &set);
                        }

                        if (strncmp(message, "force-stop", 10) == 0) {

                            CONNECTION = 0;
                            broadcast();

                            break;
                        }
                    
                    // Se il file descriptor che sto iterando è la pipe utilizzata per reinserire i file descriptor
                    } else if (fd == pipeHandleClient[0]) { 
                
                        int fileDescriptor;

                        if (read(pipeHandleClient[0], &fileDescriptor, sizeof(int)) == -1) {
                            perror("ERRORE PIPE\n");
                        }

                        if (fileDescriptor == -1) {
                            connectedClients--;
                        } else {

                            FD_SET(fileDescriptor, &set);
                            
                            if (fileDescriptor > fd_num) {
                                fd_num = fileDescriptor;
                            }
                        }

                    // Altrimenti significa che un client ha inviato un pacchetto al server  
                    } else {
                        
                        pushPacket(fd);
                        FD_CLR(fd, &set);
                        fd_num = aggiorna(set, fd_num);
                    }
                }
            }
        } 
    }
    
    close(pipeHandleConnection[0]);
    close(pipeHandleClient[0]);

    return NULL;
}


// Creo un Buffer per leggere il messaggio inviato dal client
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