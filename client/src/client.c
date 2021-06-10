#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <unistd.h>

#include "../include/client_network.h"
#include "../include/utils.h"

#define STRING_SIZE 100

// Metodi per leggere gli argomenti passati da linea di comando
void read_arguments(int argc, char *argv[]);
void handle_socket_connection(char *socketName);
void handle_write_dir(char *optarg);
void handle_write_files(char *optarg);
void handle_read_files(char *optarg);

// 
void read_all_directories(char *dirName);
void read_directory(int filesAmount);

int main(int argc, char *argv[]) {

    if (argc == 1) {
        fprintf(stderr, "Nessun argomento passato per parametro! Digita -h per vedere i comandi disponibili.\n");
        return EXIT_FAILURE;
    }

    read_arguments(argc, argv);

    return EXIT_SUCCESS;
}

void read_arguments(int argc, char *argv[]) {
 
    int opt;
    while ((opt = getopt(argc, argv, ": f: w: W: h")) != -1) {
        switch (opt) {
            case 'h':
                printf("-f filename : specifica il nome del socket AF_UNIX a cui connettersi\n");
                printf("-w dirname : invia al server i file nella cartella 'dirname'\n");
                break;

            case 'f':
                handle_socket_connection(optarg);
                break;
            case 'w':
                handle_write_dir(optarg);
                break;
            case 'W':
                handle_write_files(optarg);
                break;
            case 'r':
                handle_read_files(optarg);
                break;

            case ':':
                if (optopt == 'w') {                    
                    handle_write_dir(0);
                    break;
                }

                printf("l'opzione '-%c' richiede un argomento\n", optopt);
                break;
            case '?': 
                printf("l'opzione '-%c' non e' gestita\n", optopt);
                break;
            default:
                break;
        }
    }
}

void handle_socket_connection(char *socketName) {
    struct timespec abstime;
    
    openConnection(socketName, 1000, abstime);
}

void handle_write_dir(char *optarg) {

    // Controllo se mi è stato passato il numero di file da leggere oltre che alla directory
    char *filesAmount = strchr(optarg, ',');

    // Se non mi è stato passato alcun numero, devo leggere tutti i file presenti nella direcotry
    if (filesAmount == NULL) {
        read_all_directories(optarg);
        return;
    }

    // Se mi è stato passato un numero oltre che alla direcotry
    long nFiles;
    // Controllo se quello che mi è stato passato dopo la ',' sia effettivamente un numero
    if (isNumber(filesAmount, &nFiles) == 1) {
        fprintf(stderr, "L'argomento passato per parametro deve essere un numero! Digita -h per vedere i comandi disponibili.\n");
        exit(EXIT_FAILURE);
    }

    // Se il numero passato corrisponde a 0, allora devo leggere tutti i file presenti nella direcotry
    if (nFiles == 0) {
        read_all_directories(optarg); //TODO optarg - filesAmount  
        return;
    }

    // Altrimenti, leggo N files dalla directory
    read_directory((int) nFiles);
}

void handle_write_files(char *optarg) {

    char *token = strtok(optarg, ",");
    while (token) {
        
        openFile(token, 1);

        writeFile(token, NULL);

        token = strtok(NULL, ",");
    }
}

void handle_read_files(char *files) {

}

// ====================

void printattr(char *path) {
    struct stat info;

    if (stat(path, &info) == -1) { 
        fprintf(stderr, "Errore nella lettura del File!\n");
        exit(EXIT_FAILURE);
    } else if (S_ISREG(info.st_mode)) {
        printf("FILE!\n");
    } else if (S_ISDIR(info.st_mode)) {
        printf("DIRECTORY! %s\n", path);
    }
}
    

void read_all_directories(char *dirName) {

    DIR *directory = NULL;
    if ((directory = opendir(dirName)) == NULL) {
        perror("ERRORE: apertura della directory");
        exit(errno);
    }

    struct dirent* file;

    while ((errno = 0, file = readdir(directory)) != NULL) {
        printattr(file->d_name); 
    }

    if (errno != 0) {
        perror("ERRORE: lettura della directory");
        exit(errno);
    } else {
        if ((closedir(directory) == -1)) {
            perror("ERRORE: chiusura della directory");
            exit(EXIT_FAILURE);
        }
    }
    
}

void read_directory(int filesAmount) {

}