#ifndef SERVER_NETWORK_HANDLER_H
#define SERVER_NETWORK_HANDLER_H

#define N 100

int CONNECTION;
int STOP;

// Aggiungo un pacchetto alla lista dei pacchetti
void pushPacket(int fileDescriptor);

// Rimuove un pacchetto dalla lista dei pacchetti
int popPacket();

// Restituisce la dimensione della lista dei pacchetti
int packetQueue();

// Risveglia tutti i thread in attesa
void broadcast();

#endif