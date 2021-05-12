#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/server_config.h"
#include "../include/server_storage.h"

typedef struct File {
    char *filePath;
    char *fileContent;
    size_t fileSize;
    unsigned int modified;
} File;


int main(void) {

    printf("Avvio del server...\n\n");   

    // Carico il file config
    get_file_config();   

    // Creo il server storage
    create_storage(STORAGE_FILE_CAPACITY, STORAGE_CAPACITY);

    insert_storage("ciao.txt", "ciao!");
    // insert_storage("culorosso.txt", "ciao222!");
    // insert_storage("ciao.txt", "dasdsadsa!");

    // insert_storage("ciao.txt", "bob!");
    
    print_storage();
    destroy_storage();
      
    return EXIT_SUCCESS;
}

