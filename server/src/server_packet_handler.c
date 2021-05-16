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
    LOCK(&mutex);
    add_tail(&packetBuffer, &fileDescriptor);
    UNLOCK(&mutex);
}

// Rimuove un pacchetto dalla lista dei pacchetti
int popPacket() {    
    LOCK(&mutex);
    int *fileDescriptor = remove_head(&packetBuffer);
    UNLOCK(&mutex);
    return *fileDescriptor;
}

// Restituisce la dimensione della lista dei pacchetti
int packetQueue() {
    LOCK(&mutex);
    int length = size(packetBuffer);
    UNLOCK(&mutex);
    return length;
}