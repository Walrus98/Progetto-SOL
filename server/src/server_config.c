#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../../core/include/utils.h"
#include "../include/server_config.h"

#ifndef CONFIG_PATH
#define CONFIG_PATH "build/config.txt"
#endif

void get_file_config() {

    printf("Carico i file config:\n");

    FILE *configFile = NULL;
    if ((configFile = fopen(CONFIG_PATH, "r")) == NULL) {
        perror("ERRORE: apertura del file config.txt");
        exit(errno);
    }

    long size;
    char buffer[STRING_SIZE];
    char config[STRING_SIZE];
    char value[STRING_SIZE];

    while (fgets(buffer, STRING_SIZE, configFile)) {
        char *token = strtok(buffer, ":");
        while (token) { 

            strncpy(config, token, STRING_SIZE);
            token = strtok(NULL, ":");
            strncpy(value, token, STRING_SIZE);

            value[strlen(value) - 1] = '\0';

            printf("%s: ", config);
            printf("%s\n", value);
            
            if (strcmp(config, "storage_file_capacity") == 0) {
                if (!isNumber(value, &size)) {
                    fprintf(stderr, "ERRORE: Il valore passato dal config non è un intero!");
                    exit(-1);
                }
                STORAGE_FILE_CAPACITY = (size_t) size;
                break;
            }

            if (strcmp(buffer, "storage_capacity") == 0) { 
                if (!isNumber(value, &size)) {
                    fprintf(stderr, "ERRORE: Il valore passato dal config non è un intero!");
                    exit(-1);
                }
                STORAGE_CAPACITY = (size_t) size * 1000 * 1000;
                break;
            }

            if (strcmp(buffer, "thread_workers_amount") == 0) { 
                if (!isNumber(value, &size)) {
                    fprintf(stderr, "ERRORE: Il valore passato dal config non è un intero!\n");
                    exit(-1);
                }
                THREAD_WORKERS_AMOUNT = (size_t) size;
                break;
            }
                  
            if (strcmp(buffer, "socket_file_path") == 0) {
                SOCKET_FILE_PATH = (char *) malloc(sizeof(char) * strlen(value) + 1);
                strncpy(SOCKET_FILE_PATH, value, strlen(value) + 1);
                break;
            }

            break;
        }
    }


    
    // // Variabile che contiene il numero parsato dalla stringa
    // long size;
    // // Buffer utilizzato per leggere il file config
    // char buffer[STRING_BUFFER_SIZE];
    // // Continuo a leggere finché la chiamata della funzione non ritorna NULL
    // while (fgets(buffer, STRING_BUFFER_SIZE, configFile)) {

    //     printf("%s", buffer);

    //     // Parso la stringa caricata nel buffer
    //     char *parsedString = strchr(buffer, ':');
    //     // Visto che la stringa parsata oltre al valore interessato contiene : e uno spazio, sposto il puntatore della stringa di 2 posizioni in avanti
    //     char *configValue = (parsedString + 2);
    //     // Sostituisco il carattere di newline (ottenuto da fgets) con il carattere di terminazione
    //     configValue[strlen(configValue) - 1] = '\0';

    //     // configValue contiene quindi il valore parsato dalla stringa.
        
    //     // Rimuovo dalla stringa caricata nel buffer il valore, visto che è già salvato nella variabile configValue
    //     buffer[strlen(buffer) - strlen(parsedString)] = '\0';
        
    //     // buffer contiene quindi il nome della configurazione
    
    //     // Confronto il nome della stringa con il nome della configurazione
    //     if (strcmp(buffer, "storage_file_capacity") == 0) {
    //         // Se il valore passato dal config è un numero
    //         if (isNumber(configValue, &size)) {
    //             // Assegno il valore alla variabile globale
    //             STORAGE_FILE_CAPACITY = (size_t) size;
    //             continue;
    //         } 
    //         fprintf(stderr, "ERRORE: storage_file_capacity vuole un intero!");
    //         exit(EXIT_FAILURE);
    //     }

    //     // Confronto il nome della stringa con il nome della configurazione
    //     if (strcmp(buffer, "storage_capacity") == 0) {
    //         // Se il valore passato dal config è un numero
    //         if (isNumber(configValue, &size)) {
    //             // Assegno il valore alla variabile globale
    //             STORAGE_CAPACITY = (size_t) size;
    //             continue;
    //         } 
    //         fprintf(stderr, "ERRORE: storage_capacity vuole un intero!");
    //         exit(EXIT_FAILURE);
    //     }

    //     // Confronto il nome della stringa con il nome della configurazione
    //     if (strcmp(buffer, "replacement_policy") == 0) {
    //         // Se il valore passato dal config è un numero
    //         if (isNumber(configValue, &size)) {
    //             // Assegno il valore alla variabile globale
    //             REPLACEMENT_POLICY = (int) size;
    //             continue;
    //         } 
    //         fprintf(stderr, "ERRORE: storage_capacity vuole un intero!");
    //         exit(EXIT_FAILURE);
    //     }

    //     // Confronto il nome della stringa con il nome della configurazione
    //     if (strcmp(buffer, "thread_workers_amount") == 0) {
    //         // Se il valore passato dal config è un numero
    //         if (isNumber(configValue, &size)) {
    //             // Assegno il valore alla variabile globale
    //             THREAD_WORKERS_AMOUNT = (size_t) size;
    //             continue;
    //         } 
    //         fprintf(stderr, "ERRORE: thread_workers_amount vuole un intero!");
    //         exit(EXIT_FAILURE);
    //     }

    //     // // Confronto il nome della stringa con il nome della configurazione
    //     // if (strcmp(buffer, "socket_file_name") == 0) {

    //     //     // Alloco la memoria per contenere la stringa passata dal config
    //     //     if ((SOCKET_FILE_NAME = malloc(sizeof(char *) * strlen(buffer))) == NULL)  {
    //     //         perror("ERRORE: impossibile allocare la memoria richiesta per la stringa socket_file_name");
    //     //         exit(errno);
    //     //     }

    //     //     // Copio il contenuto della stringa nella variabile 
    //     //     strncpy(SOCKET_FILE_NAME, configValue, STRING_BUFFER_SIZE);
    //     //     continue;

    //     //     fprintf(stderr, "ERRORE: mb_cache_size vuole un intero!");
    //     // }
    // }
    printf("\n");

    
    // Chiudo il descrittore del file
    if (fclose(configFile) != 0) {
        perror("ERRORE: chiusura del file config.txt");
        exit(errno);
    }   

    // printf("Valori ottenuti:\n");
    // printf("%ld\n", THREAD_WORKERS_AMOUNT);
    // printf("%ld\n", STORAGE_SIZE);
    // printf("%s\n", SOCKET_FILE_NAME);

}