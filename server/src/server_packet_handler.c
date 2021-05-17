#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/list_utils.h"
#include "../include/pthread_utils.h"

static List packetBuffer;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

// Aggiungo un pacchetto alla lista dei pacchetti
void pushPacket(int fileDescriptor) {    
    
    // Acquisco la lock sul buffer
    LOCK(&mutex);

    // ALloco sull'heap il descrittore del file, così da avere l'indirizzo di memoria condiviso fra più thread
    int *fd = (int *) malloc(sizeof(int));
    *fd = fileDescriptor;

    // Aggiungo la richiesta sul buffer
    add_tail(&packetBuffer, fd);

    // printf("HO AGGIUNTO NELLA CODA %d\n", *fd);

    // Segnalo ai thread worker che c'è un elemento all'interno del buffer
    SIGNAL(&cond);

    // Rilascio la lock sul buffer
    UNLOCK(&mutex);
}

// Rimuove un pacchetto dalla lista dei pacchetti
int popPacket() {

    // Acquisco la lock sul buffer
    LOCK(&mutex);

    // Se il buffer non contiene nessuna richiesta
    int *fd;
    while ((fd = ((int *) remove_head(&packetBuffer))) == NULL) {
        // Metto il thread in stato di wait
        WAIT(&cond, &mutex);
    }
    
    // Assegno il contenuto della variabile condivisa a fileDescriptor
    int fileDescriptor = *fd;

    // printf("HO PRESO DALLA CODA %d\n", fileDescriptor);

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