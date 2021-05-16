#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_config.h"
#include "../include/server_storage.h"
#include "../include/server_network.h"

int main(void) {

    printf("Avvio del server...\n\n");   

    // Carico il file config
    get_file_config();   

    // Creo il server storage
    create_storage(STORAGE_FILE_CAPACITY, STORAGE_CAPACITY, REPLACEMENT_POLICY);

    // Stabilisco la connessione con i client
    create_connection();

    // insert_storage("ciao.txt", "messaggiodiprova");
    // insert_storage("123213.txt", "cambiato");
    // insert_storage("prova.txt", "dddd");

    // Stabilisco la connessione
    // establish_connection();


    // insert_storage("ciao.txt", "messaggiodiprova");
    // insert_storage("123213.txt", "cambiato");
    // insert_storage("prova.txt", "dddd");
    
    // print_storage();

    destroy_storage();
      
    return EXIT_SUCCESS;
}

