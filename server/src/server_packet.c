#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>

void *deserialize(void *packet) {


}

void serialize(void *packet, int id, int size, char *message) {

}

int main() {

    int id = INT_MAX;
    char message[5] = "ciac"; // c i a o \0
    int size = strlen(message) + 1 + sizeof(int);

    void *buffer = malloc(size);

    // serialize
    memcpy(buffer, &id, sizeof(int));
    memcpy(buffer + sizeof(int), message, strlen(message));


    //deserialize
    int ciao;
    memcpy(&ciao, buffer, sizeof(int));

    char messaggio2[5];
    memcpy(messaggio2, buffer + sizeof(int), strlen(message));


    printf("%d\n", ciao);
    printf("%s\n", messaggio2);






    // printf("Here is the message: %ld\n", sizeof(id));
    // for (int i = 0; i < size; i++) {
    //     printf("%#x ",  buffer[i]); // *(buffer + i)
    // }

    // serialize(packet, id, size, message);

    return 0;
}