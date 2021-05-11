#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_config.h"
#include "../include/server_storage.h"

int main(void) {

    printf("Avvio del server...\n\n");   

    // Carico il file config
    get_file_config();   

    // Creo il server storage
    create_file_storage(STORAGE_FILE_CAPACITY, STORAGE_CAPACITY);

    
    
    
    return EXIT_SUCCESS;
}

