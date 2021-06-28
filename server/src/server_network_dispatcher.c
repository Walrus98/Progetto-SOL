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
#include "../include/server_config.h"
#include "../../core/include/utils.h"

#define UNIX_PATH_MAX 108 

// Aggiorna il valore di fd_max
static int update(fd_set set, int fd_max) {
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

    // Prendo gli argomenti passati per parametro
    DispatcherArg *argument = (DispatcherArg *) dispatcherArgument;
    
    // Pipe per svegliare il Thread Dispatcher quando deve terminare
    pipeHandleConnection[0] = argument->pipeHandleConnection[0];
    pipeHandleConnection[1] = argument->pipeHandleConnection[1];

    // Pipe per ricevere il file descriptor del client quando il Thread Worker ha terminato il Task
    pipeHandleClient[0] = argument->pipeHandleClient[0];
    pipeHandleClient[1] = argument->pipeHandleClient[1];

    // Creo la struct socket e la imposto come AF_UNIX
    struct sockaddr_un sa; 
    strncpy(sa.sun_path, SOCKET_FILE_PATH, UNIX_PATH_MAX);
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

    // Assegno a fd_num il fd più grande tra le pipe e il server socket
    int fd_num;
    if (fd_sk >= pipeHandleConnection[0] && fd_sk >= pipeHandleClient[0]) {
        fd_num = fd_sk;
    } else if (pipeHandleConnection[0] >= pipeHandleClient[0] && pipeHandleConnection[0] >= fd_sk) {
        fd_num = pipeHandleConnection[0];
    } else {
        fd_num = pipeHandleClient[0];
    }

    // Finché la variabile globale della connessione è settata a 1
    while (CONNECTION == 1) {
        
        // Copio in rdset i valori contenuti in set
        rdset = set;

        // Se ricevo un segnale di SIGHUP e non ho più client connessi
        if (connectedClients == 0 && ACCEPT_CONNECTION == 0) {        
            // Imposto la variabile globale della connessione a 0    
            CONNECTION = 0;
            // Risveglio tutti i thread worker ed esco dal ciclo
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

                        // Aumento il numero di client connessi, questa variabile mi serve nel caso di ricezione di un SIGHUP
                        connectedClients++;

                    // Se il file descriptor che sto iterando è la pipe di terminazione del Thread Dispatcher
                    } else if (fd == pipeHandleConnection[0]) {

                        char message[10];

                        // Leggo il contenuto del messaggio inviato tramite la pipe
                        int nread = readn(pipeHandleConnection[0], &message, 10);
                        if (nread == 0 || nread == -1) {
                            perror("ERRORE PIPE\n");
                            exit(errno);
                        }

                        // Se il messaggio inviato tramite pipe contiene stop, allora ho ricevuto un SIGHUP
                        if (strncmp(message, "stop", 10) == 0) {
                            // Non accetto più nuove connessioni
                            ACCEPT_CONNECTION = 0;
                            // Rimuovo dal set la pipe
                            FD_CLR(pipeHandleConnection[0], &set);
                        }

                        // Se il messaggio inviato tramite pipe contiene force-stop, allora ho ricevuto un SIGINT o SIGQUIT
                        if (strncmp(message, "force-stop", 10) == 0) {

                            // Devo terminare subito il server, quindi imposto la variabile globale della connessione a 0
                            CONNECTION = 0;
                            // Risveglio tutti i Thread Worker
                            broadcast();

                            break;
                        }
                    
                    // Se il file descriptor che sto iterando è la pipe utilizzata per reinserire i file descriptor una volta che
                    // il Thread Worker ha terminato il task
                    } else if (fd == pipeHandleClient[0]) { 
                
                        int fileDescriptor;

                        // Leggo il contenuto del messaggio inviato tramite la pipe
                        int nread = readn(pipeHandleClient[0], &fileDescriptor, sizeof(int));
                        if (nread == 0 || nread == -1) {
                            perror("ERRORE PIPE\n");
                            exit(errno);
                        }

                        // Se il valore passato dal thread worker corrisponde a -1, significa che un utente si è disconnesso e quindi non devo
                        // inserire nuovamente il fd nel set
                        if (fileDescriptor == -1) {
                            // Diminuisco il numero di client connessi, mi serve per il segnale di SIGHUP
                            connectedClients--;
                        } else {
                            // Altrimenti inserisco nuovamente il fd nel set
                            FD_SET(fileDescriptor, &set);
                            
                            // Se il fd che sto inserendo è maggiore di fd_num, aggiorno fd_num
                            if (fileDescriptor > fd_num) {
                                fd_num = fileDescriptor;
                            }
                        }

                    // Altrimenti significa che un client ha inviato un pacchetto al server 
                    } else {
                        // Inserisco il fd nella lista di task dei thread worker
                        pushPacket(fd);
                        // Rimuovo dal set il fd dell'utente
                        FD_CLR(fd, &set);
                        // Aggiorno il valore di fd_num visto che ho rimosso un fd dal set
                        fd_num = update(set, fd_num);
                    }
                }
            }
        } 
    }
    
    // Chiudo i pipe
    close(pipeHandleConnection[0]);
    close(pipeHandleClient[0]);

    return NULL;
}