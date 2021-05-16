#ifndef SERVER_PACKET_HANDLER_H
#define SERVER_PACKET_HANDLER_H

// Aggiungo un pacchetto alla lista dei pacchetti
void pushPacket(int fileDescriptor);

// Rimuove un pacchetto dalla lista dei pacchetti
int popPacket();

// Restituisce la dimensione della lista dei pacchetti
int packetQueue();

#endif