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

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Numero di argomenti non valido!\n");
        return EXIT_FAILURE;
    }

    if ((CONFIG_PATH = (char *) malloc(strlen(argv[1]) + 1)) == NULL) {
        perror("ERRORE: Impossibile allocare memoria richiesta.");
        return EXIT_FAILURE;
    }
    strncpy(CONFIG_PATH, argv[1], strlen(argv[1]) + 1);

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

    free(CONFIG_PATH);

    printf("\n");
      
    return EXIT_SUCCESS;
}

