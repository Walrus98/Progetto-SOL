#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../../core/include/utils.h"
#include "../include/server_config.h"

void get_file_config() {

    printf("Carico i file config:\n");

    // Prendo il descrittore del file, se all'avvio del server non definisco CONFIG_PATH, prendo come config di default quello presente in build/config.txt
    FILE *configFile = NULL;
    if ((configFile = fopen(CONFIG_PATH, "r")) == NULL) {
        perror("ERRORE: apertura del file config.txt");
        exit(errno);
    }

    long size;
    char buffer[STRING_SIZE];
    char config[STRING_SIZE];
    char value[STRING_SIZE];
    
    // Leggo riga per riga il file config
    while (fgets(buffer, STRING_SIZE, configFile)) {
        // Parso la riga letta dal file config
        char *token = strtok(buffer, ":");
        while (token) { 
            // Copio in config il testo che sta prima dei :
            strncpy(config, token, STRING_SIZE);
            // Parso la stringa
            token = strtok(NULL, ":");
            // Copio in value il testo che sta dopo i :
            strncpy(value, token, STRING_SIZE);
            // Inserisco il carattere di terminazione, altrimenti la funzione isNumber non funziona correttamente
            value[strlen(value) - 1] = '\0';

            // Il nome della configurazione
            printf("%s: ", config);
            // Il valore passato come configurazione
            printf("%s\n", value);
            
            // Se il config che sto leggendo corrisponde a storage_file_capacity
            if (strcmp(config, "storage_file_capacity") == 0) {
                // Controllo se il valore passato dall'utente è un numero
                if (!isNumber(value, &size)) {
                    fprintf(stderr, "ERRORE: Il valore passato dal config non è un intero!");
                    exit(-1);
                }
                // Assegno alla variabile globale il valore letto da config
                STORAGE_FILE_CAPACITY = (size_t) size;
                break;
            }

            // Se il config che sto leggendo corrisponde a storage_capacity
            if (strcmp(buffer, "storage_capacity") == 0) { 
                // Controllo se il valore passato dall'utente è un numero
                if (!isNumber(value, &size)) {
                    fprintf(stderr, "ERRORE: Il valore passato dal config non è un intero!");
                    exit(-1);
                }
                // Assegno alla variabile globale il valore convertito in Megabyte
                STORAGE_CAPACITY = (size_t) size * 1024 * 1024;
                break;
            }

            // Se il config che sto leggendo corrisponde a thread_workers_amount
            if (strcmp(buffer, "thread_workers_amount") == 0) { 
                // Controllo se il valore passato dall'utente è un numero
                if (!isNumber(value, &size)) {
                    fprintf(stderr, "ERRORE: Il valore passato dal config non è un intero!\n");
                    exit(-1);
                }
                // Assegno alla variabile globale il valore letto da config
                THREAD_WORKERS_AMOUNT = (size_t) size;
                break;
            }
                  
            // Se il config che sto leggendo corrisponde a socket_file_path
            if (strcmp(buffer, "socket_file_path") == 0) {
                // Alloco nello heap la memoria necessaria per contenere il nome del file passato per file config
                if ((SOCKET_FILE_PATH = (char *) malloc(sizeof(char) * strlen(value) + 1)) == NULL) {
                    perror("ERRORE: Impossibile allocare la memoria richiesta");
                    return;
                }
                // Copio il contenuto della stringa nella stringa appena creata
                strncpy(SOCKET_FILE_PATH, value, strlen(value) + 1);
                break;
            }

            break;
        }
    }

    printf("\n");

    // Chiudo il descrittore del file
    if (fclose(configFile) != 0) {
        perror("ERRORE: chiusura del file config.txt");
        exit(errno);
    }   

}