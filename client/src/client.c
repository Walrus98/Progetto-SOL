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
#include "../include/list_utils.h"
#include "../../core/include/utils.h"

// Metodi per leggere gli argomenti passati da linea di comando
static void read_arguments(int argc, char *argv[]);
static void addArgument(char c, char *arg);
static void execute_arguments();
static void free_arguments();

// Metodi per gestire le richieste dei client
static void handle_socket_connection(char *socketName);
static void handle_write_dir(char *optarg);
static void handle_write_files(char *optarg);
static void handle_read_files(char *optarg);
static void handle_read_n_files(char *optarg);
static void handle_remove_file(char *optarg);
static void handle_help_commands();

// Metodi aggiuntivi 
static void check_attribute(char *path, int n);
static void read_directories(char *dirName, int n);
static void write_file_directory(char *dirName, char *fileName, char *buffer);

// Lista di Argomenti
static Node *argumentList;

typedef struct Argument {
    char *command;
    char *argument;
} Argument;

static int fun_compare(void *a, void *b) {
    Argument *arg = (Argument *) a;
    char command = *((char *) b);

    return *(arg->command) == command;
}

static int TIME;
static char *DIRNAME = NULL;

int main(int argc, char *argv[]) {

    if (argc == 1) {
        fprintf(stderr, "Nessun argomento passato per parametro! Digita -h per vedere i comandi disponibili.\n");
        return EXIT_FAILURE;
    }

    printf("Avvio del client in corso...\n\n");

    create_list(&argumentList, fun_compare);

    read_arguments(argc, argv);
    execute_arguments();
    free_arguments();

    return EXIT_SUCCESS;
}

void read_arguments(int argc, char *argv[]) {
 
    int opt;
    while ((opt = getopt(argc, argv, ": f: w: W: r: R: d: c: h p")) != -1) {
        switch (opt) {
            case ':':
                printf("l'opzione '-%c' richiede un argomento\n", optopt);
                break;
            case '?': 
                printf("l'opzione '-%c' non e' gestita\n", optopt);
                break;
            default:
                addArgument(opt, optarg);
        }
    }
}

void addArgument(char c, char *arg) {

    Argument *newArg;
    if ((newArg = (Argument *) malloc(sizeof(Argument))) == NULL) {
        perror("ERRORE: impossibile allocare memoria per Argument.");
        exit(errno);
    }

    char *command;
    if ((command = (char *) malloc(sizeof(char))) == NULL) {
        perror("ERRORE: impossibile allocare memoria per il comando.");
        exit(errno);
    }

    *command = c;
    
    char *argument = NULL;
    if (arg != NULL) {
        if ((argument = (char *) malloc(sizeof(char) * strlen(arg) + 1)) == NULL) {
            perror("ERRORE: impossibile allocare memoria per l'argomento.");
            exit(errno);
        }
        strncpy(argument, arg, strlen(arg) + 1);
    } 

    newArg->command = command;
    newArg->argument = argument;

    add_tail(&argumentList, newArg);
}

void execute_arguments() {

    Argument *arg;
    char command;

    command = 'h';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        handle_help_commands();
        remove_value(&argumentList, arg);
    }

    command = 'p';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        DEBUG_ENABLE = 1;
        DEBUG("Modalità di DEBUG attivata.\n");
        remove_value(&argumentList, arg);
    }

    command = 't';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        char *argument = arg->argument;
        long value;
        if (isNumber(argument, &value) == 1) {
            TIME = value; 
            remove_value(&argumentList, arg);
        } else {
            fprintf(stderr, "L'argomento passato per parametro deve essere un numero! Digita -h per vedere i comandi disponibili.\n");
            return;
        }
    }
    
    command = 'd';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        char command_r = 'r';
        char command_R = 'R';
        if (get_value(argumentList, &command_r) != NULL || get_value(argumentList, &command_R) != NULL ) {
            char *directory = arg->argument;

            if ((DIRNAME = (char *) malloc(sizeof(char) * strlen(directory) + 1)) == NULL) {
                perror("ERRORE: impossibile allocare memoria per DIRNAME.");
                exit(errno);
            }
            strncpy(DIRNAME, directory, strlen(directory) + 1);

            remove_value(&argumentList, arg);
        } else {
            fprintf(stderr, "Impossibile eseguire il comando -d senza eseguire anche un comando -R o -r!\n");
            return;
        }
    }
     
    command = 'f';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        printf("Connessione al socket: \"%s\".\n", arg->argument);
        handle_socket_connection(arg->argument);
        remove_value(&argumentList, arg);
    } else {
        fprintf(stderr, "Nessuna connessione stabilita! Digita -h per vedere i comandi disponibili.\n");
        return;
    }

    for (Node *curr = argumentList; curr != NULL; curr = curr->next) {
        Argument *arg = (Argument *) curr->value;
        char command = *(arg->command);
        char *argument = arg->argument;
        switch (command) {
            case 'w':
                handle_write_dir(argument);
                break;
            case 'W':
                handle_write_files(argument);
                break;
            case 'r':
                handle_read_files(argument);
                break;
            case 'R':
                handle_read_n_files(argument);
                break;
            case 'c':
                handle_remove_file(argument);
                break;
            default:
                break;
        }
    }

    closeConnection(SOCKET_PATH);  
}

void free_arguments() {
    
    for (Node *curr = argumentList; curr != NULL;) {
        Argument *arg = (Argument *) curr->value;

        Node *temp = curr;

        // Cancello il valore della lista
        free(arg->command);
        free(arg->argument);
        free(arg);

        curr = curr->next;
        
        // Cancello il nodo
        free(temp);
    }

    if (DIRNAME != NULL) {
        free(DIRNAME);
    }

    if (SOCKET_PATH != NULL) {
        free(SOCKET_PATH);
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

        argument[size] = (char *) malloc(length);
        strncpy(argument[size], token, length);

        token = strtok(NULL, ",");
    }

    // for (int i = 0; i < size; i++) {
    //     printf("%s\n", argument[i]);
    // }
    
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

        if (openFile(token, O_CREATE) == -1) openFile(token, NO_ARG);
        writeFile(token, NULL);
        closeFile(token);

        token = strtok(NULL, ",");
    }
}

void handle_read_files(char *optarg) {

    size_t bufferSize;
    void *buffer = NULL;

    char *token = strtok(optarg, ",");
    while (token) {

        openFile(token, NO_ARG);

        if (readFile(token, &buffer, &bufferSize) == 0) {

            printf("Messaggio ricevuto: %s\n", (char *) buffer);

            if (DIRNAME != NULL) {
                write_file_directory(DIRNAME, token, (char *) buffer);
            }

            free(buffer);
        }

        closeFile(token);

        token = strtok(NULL, ",");
    }
}

void handle_read_n_files(char *optarg) {
    if (optarg == NULL) {
        readNFiles(0, DIRNAME);
    } 

    long nFiles;
    if (isNumber(optarg, &nFiles)) {
        readNFiles((int) nFiles, DIRNAME);
    } else {
        fprintf(stderr, "L'argomento passato per parametro deve essere un numero! Digita -h per vedere i comandi disponibili.\n");
    }
}

void handle_remove_file(char *optarg) {
    char *token = strtok(optarg, ",");
    while (token) {

        if (openFile(token, O_CREATE) == -1) openFile(token, NO_ARG);
        removeFile(token);

        token = strtok(NULL, ",");
    }
}

void handle_help_commands() {
    printf("-f filename : specifica il nome del socket AF_UNIX a cui connettersi\n");
    printf("-w dirname : invia al server i file nella cartella 'dirname'\n");
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
        
        openFile(path, O_CREATE);
        writeFile(path, NULL);
        closeFile(path);

        cont++;
    } else if (S_ISDIR(info.st_mode)) {
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
        perror("ERRORE: impossibile prendere la directory corrente"); 
        exit(EXIT_FAILURE);
    }

    if (chdir(dirName) == -1) {
        perror("ERRORE: impossibile spostarsi nella directory"); 
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
        perror("ERRORE: impossibile spostarsi nella directory"); 
        exit(EXIT_FAILURE);
    }    
}

void write_file_directory(char *dirName, char *fileName, char *buffer) {

    if (chdir(dirName) == -1) {
        perror("ERRORE: impossibile spostarsi nella directory");
        exit(errno);
    }

    char directory[STRING_SIZE];
    if (getcwd(directory, STRING_SIZE) == NULL) {
        perror("ERRORE: impossibile prendere la directory corrente");
        exit(errno);
    }

    char *token = strtok(fileName, "/"); 
    char *fName = NULL;
    while (token) {
        fName = token;
        token = strtok(NULL, "/");
    }

    if (fName != NULL) {
        strcat(directory, "/");
        strcat(directory, fName);
    } else {
        strcpy(directory, fileName);
    }

    FILE *file = NULL;
    if ((file = fopen(directory, "w")) == NULL) {
        perror("ERRORE: impossibile aprire il file");
        exit(errno);
    } 

    if (fprintf(file, "%s", buffer) < 0) {
        perror("ERRORE: impossibile scrivere il file");
        exit(errno);
    }

    if (fclose(file) != 0) {
        perror("ERRORE: impossibile chiudere il file");
        exit(errno);
    }
}