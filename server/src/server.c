#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_config.h"
#include "../include/server_storage.h"
#include "../include/server_network.h"
#include "../../core/include/utils.h"

int DEBUG_ENABLE = 0;

int main(void) {

    printf("Avvio del server...\n\n");   

    // Carico il file config
    get_file_config();   

    // Creo il server storage
    create_storage(STORAGE_FILE_CAPACITY, STORAGE_CAPACITY); //REPLACEMENT_POLICY

    // Stabilisco la connessione con i client
    create_connection(THREAD_WORKERS_AMOUNT);

    destroy_storage();

    free(SOCKET_FILE_PATH);

    printf("\n");
      
    return EXIT_SUCCESS;
}

