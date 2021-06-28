#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "../../core/include/list_utils.h"
#include "../include/pthread_utils.h"
#include "../include/server_network_handler.h"

static List packetBuffer = NULL;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;

// Aggiungo un pacchetto alla lista dei pacchetti
void pushPacket(int fileDescriptor) {    
    
    // Acquisco la lock sul buffer
    LOCK(&mutex);

    // ALloco sull'heap il descrittore del file, così da avere l'indirizzo di memoria condiviso fra più thread
    int *fd;
    if ((fd = (int *) malloc(sizeof(int))) == NULL) {
        perror("ERRORE: Impossibile allocare la memoria richiesta");
        exit(errno);
    }
    *fd = fileDescriptor;

    // Aggiungo la richiesta sul buffer
    add_tail(&packetBuffer, fd);

    // Segnalo ai thread worker che c'è un elemento all'interno del buffer
    SIGNAL(&cond);

    // Rilascio la lock sul buffer
    UNLOCK(&mutex);
}

// Rimuove un pacchetto dalla lista dei pacchetti
int popPacket() {

    // Acquisco la lock sul buffer
    LOCK(&mutex);

    int *fd;
    // Metto in attesa il thread worker finché non riceve un task dalla lista thread safe o la connessione non è settata a 0
    while ((fd = ((int *) remove_head(&packetBuffer))) == NULL && CONNECTION == 1) {
        // Metto il thread in stato di wait
        WAIT(&cond, &mutex);
    }

    // Se la connessione è 0, rilascio la lock e restituisco -1, così termino il Thread Worker
    if (CONNECTION == 0) {
        UNLOCK(&mutex);
        return -1;
    }
    
    // Assegno il contenuto della variabile condivisa a fileDescriptor
    int fileDescriptor = *fd;

    // Cancello il descrittore del file dal buffer
    free(fd);

    // Il thread rilascia la lock e restituisce il filedescriptor
    UNLOCK(&mutex);

    return fileDescriptor;
}

// Restituisce la dimensione della lista dei pacchetti
int packetQueue() {
    LOCK(&mutex);
    int length = size(packetBuffer);
    UNLOCK(&mutex);
    return length;
}

// Sveglia tutti i thread in attesa
void broadcast() {
    LOCK(&mutex);
    BCAST(&cond);
    UNLOCK(&mutex);
}