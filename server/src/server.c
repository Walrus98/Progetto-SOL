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
    create_storage(STORAGE_FILE_CAPACITY, STORAGE_CAPACITY);

    insert_storage("Pippo.txt", "asdsadsasa");
    // insert_storage("Samu.txt", "");
    // insert_storage("test.txt", "Ciao!");
    // insert_storage("sadsa.txt", "Ciao!");
    
    // print_file_storage();

    destroy_storage();
      
    return EXIT_SUCCESS;
}

