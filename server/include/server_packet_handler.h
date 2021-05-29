#ifndef SERVER_PACKET_HANDLER_H
#define SERVER_PACKET_HANDLER_H

#define OPEN_FILE 0
#define READ_FILE 1
#define READ_N_FILES 2
#define WRITE_FILE 3
#define APPEND_TO_FILE 4
#define LOCK_FILE 5
#define UNLOCK_FILE 6
#define CLOSE_FILE 6
#define REMOVE_FILE 7

void handlePacket(int packetID, int packetSize, char *payload, int fileDescriptor);

#endif