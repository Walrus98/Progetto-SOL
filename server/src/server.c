#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_config.h"
#include "../include/server_storage.h"
#include "../include/server_network.h"
#include "../../core/include/utils.h"

// Variabile globale per attivare la modalità di DEBUG del server
int DEBUG_ENABLE = 0;

// fix bash, -h, test append, read_n

int main(void) {

    printf("Avvio del server...\n\n");   

    // Carico il file config
    get_file_config();   

    // Creo il server storage
    create_storage(STORAGE_FILE_CAPACITY, STORAGE_CAPACITY);

    // Creo la connessione con i client
    create_connection(THREAD_WORKERS_AMOUNT);

    // Libero la memoria a termine esecuzione del server
    destroy_storage();
    if (SOCKET_FILE_PATH != NULL) free(SOCKET_FILE_PATH);

    printf("\n");
      
    return EXIT_SUCCESS;
}

