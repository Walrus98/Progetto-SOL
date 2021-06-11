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
static void check_attribute(char *path, int n);
static void read_directories(char *dirName, int n);


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
    while ((opt = getopt(argc, argv, ": f: w: W: r: h")) != -1) {
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
                // if (optopt == 'w') {                    
                //     handle_write_dir(0);
                //     break;
                // }

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

    char **argument = NULL;    

    int size = 0;
    char *token = strtok(optarg, ",");
    for (; token && size < 2; size++) {
        argument = realloc(argument, sizeof(char *) * (size + 1));

        int length = strlen(token) + 1;

        argument[size] = malloc(length);
        strncpy(argument[size], token, length);

        token = strtok(NULL, ",");
    }

    for (int i = 0; i < size; i++) {
        printf("%s\n", argument[i]);
    }
    
    // Se non mi ha passato un numero dopo la virgola
    if (size == 1) {
        // Leggo tutte le directory
        read_directories(optarg, 0);
    } 

    // Se non mi ha passato qualcosa dopo la virgola
    if (size == 2) {
        // Controllo se quello che mi è stato passato dopo la ',' sia effettivamente un numero
        long nFiles;
        if (isNumber(argument[1], &nFiles) == 0) {
            fprintf(stderr, "L'argomento passato per parametro deve essere un numero! Digita -h per vedere i comandi disponibili.\n");
            exit(EXIT_FAILURE);
        }

        // Se il numero passato corrisponde a 0, allora devo leggere tutti i file presenti nella direcotry
        read_directories(argument[0], nFiles);
    }
    
    for (int i = 0; i < size; i++) {
         free(argument[i]);
    }
    free(argument);

}

void handle_write_files(char *optarg) {

    char *token = strtok(optarg, ",");
    while (token) {

        openFile(token, 1);
        writeFile(token, NULL);

        token = strtok(NULL, ",");
    }
}

void handle_read_files(char *optarg) {
    char *token = strtok(optarg, ",");
    while (token) {

        openFile(token, 1);
        readFile(token, NULL, NULL);

        token = strtok(NULL, ",");
    }
}

// ====================

static int cont = 0;

void check_attribute(char *path, int n) {
    
    struct stat info;

    if (stat(path, &info) == -1) { 
        fprintf(stderr, "Errore nella lettura del File!\n");
        exit(EXIT_FAILURE);
    } 

    if (S_ISREG(info.st_mode)) {
        printf("FILE! %s\n", path);
        openFile(path, 1);
        writeFile(path, NULL);
        cont++;
    } else if (S_ISDIR(info.st_mode)) {
        printf("DIRECTORY! %s\n", path);
        read_directories(path, n);
    }
}    

void read_directories(char *dirName, int n) {

    DIR *directory = NULL;
    if ((directory = opendir(dirName)) == NULL) {
        perror("ERRORE: apertura della directory");
        exit(errno);
    }

    struct dirent* file;

    char buf[100];

    if (getcwd(buf, 100)==NULL) {
        perror("getcwd"); 
        exit(EXIT_FAILURE);
    }

    if (chdir(dirName) == -1) {
        perror("chdir"); 
        exit(EXIT_FAILURE);
    }

    while ((errno = 0, file = readdir(directory)) != NULL && (n == 0 || cont < n)) {
        
        if (strcmp("..", file->d_name) == 0 || strcmp(".", file->d_name) == 0) {
            continue;
        }
        check_attribute(file->d_name, n);
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
    if (chdir(buf) == -1) {
        perror("chdir"); 
        exit(EXIT_FAILURE);
    }    
}