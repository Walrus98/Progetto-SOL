#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <unistd.h>

#include "../include/client_network.h"
#include "../../core/include/list_utils.h"
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
// static void write_file_directory(char *dirName, char *fileName, char *buffer);

// Lista di Argomenti
static Node *argumentList;

// Struttura utilizzata all'interno della lista
typedef struct Argument {
    // Il carattere del comando (es. -f, -d, etc..)
    char *command;
    // Il contenuto del comando
    char *argument;
} Argument;

// Funzione per confrontare gli elementi all'interno della lista
static int fun_compare(void *a, void *b) {
    Argument *arg = (Argument *) a;
    char command = *((char *) b);

    return *(arg->command) == command;
}

// Tempo in millisecondi utilizzato dal comando -t
static int TIME = 0;
// Directory utilizzata dal comando -d
static char *DIRNAME = NULL;

int main(int argc, char *argv[]) {
    
    if (argc == 1) {
        fprintf(stderr, "\nNessun argomento passato per parametro! Digita -h per vedere i comandi disponibili.\n");
        return EXIT_FAILURE;
    }

    printf("\nAvvio del client in corso...\n\n");

    // Inizializzo la lista a NULL e assegno alla lista la funzione di compare definita sopra
    create_list(&argumentList, fun_compare);

    // Leggo tutti gli argomenti che l'utente ha avviato e li inserisco all'interno della lista
    read_arguments(argc, argv);
    // Eseguo i comandi contenuti all'interno della lista
    execute_arguments();
    // Libero dalla memoria principale gli argomenti nella lista
    free_arguments();

    printf("\n");
    
    return EXIT_SUCCESS;
}

// Leggo tutti gli argomenit che l'utente ha inserito all'avvio del programma e li inserisco nella lista
void read_arguments(int argc, char *argv[]) {
 
    int opt;
    while ((opt = getopt(argc, argv, ": h f: w: W: r: R: d: t: c: p")) != -1) {
        switch (opt) {
            // Se l'utente mi massa un comando senza argomento che in realtà lo richiede
            case ':':
                printf("L'opzione '-%c' richiede un argomento\n", optopt);
                break;
            // Se l'utente mi passa un comando inesistente
            case '?':
                printf("L'opzione '-%c' non è gestita. Digita -h per vedere i comandi disponibili.\n", optopt);
                break;
            default:
                addArgument(opt, optarg);
        }
    }
}

// Inserisco gli argomenti all'interno della lista
void addArgument(char c, char *arg) {

    // Alloco nello heap la struct di Argument
    Argument *newArg;
    if ((newArg = (Argument *) malloc(sizeof(Argument))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria per Argument.");
        exit(errno);
    }

    // Alloco nello heap il comando passato per parametro
    char *command;
    if ((command = (char *) malloc(sizeof(char))) == NULL) {
        perror("ERRORE: Impossibile allocare memoria per il comando.");
        exit(errno);
    }

    *command = c;
    
    // Alloco nello heap il l'argomento passato per parametro, se presente (es. -h non possiede l'argomento)
    char *argument = NULL;
    if (arg != NULL) {
        if ((argument = (char *) malloc(sizeof(char) * strlen(arg) + 1)) == NULL) {
            perror("ERRORE: Impossibile allocare memoria per l'argomento.");
            exit(errno);
        }
        strncpy(argument, arg, strlen(arg) + 1);
    } 

    // Assegno alla struct creata l'argomento e il comando
    newArg->command = command;
    newArg->argument = argument;

    // Aggiungo la struct all'interno della lista
    add_tail(&argumentList, newArg);
}

// Eseguo gli argomenti presenti all'interno della lista
void execute_arguments() {

    Argument *arg;
    char command;

    // Se la lista di argomenti contiene il comando -h
    command = 'h';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        // Eseguo il comando richiesto
        handle_help_commands();
        // Rimuovo il comando dalla lista
        remove_value(&argumentList, arg);
    }

    // Se la lista di argomenti contiene il comando -p
    command = 'p';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        // Abilito la modalità di debug
        DEBUG_ENABLE = 1;
        DEBUG(("Modalità di DEBUG attivata.\n"));
        // Rimuovo il comando dalla lista
        remove_value(&argumentList, arg);
    }

    // Se la lista di argomenti contiene il comando -t
    command = 't';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        // Prendo l'argomento che l'utente ha inserito con il comando -t
        char *argument = arg->argument;
        // Controllo se l'argomento è un numero
        long value;
        if (isNumber(argument, &value) == 1) {
            // Se è un numero, allora assegno a time il valore richiesto dall'utente
            TIME = value; 
            // Rimuovo il comando dalla lista
            remove_value(&argumentList, arg);
        } else {
            // Altrimenti restituisco un messaggio di errore
            fprintf(stderr, "ERRORE: L'argomento passato per parametro deve essere un numero! Digita -h per vedere i comandi disponibili.\n");
            return;
        }
    }
    
    // Se la lista di argomenti contiene il comando -d
    command = 'd';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        // Controllo se nella lista è presente anche il comando -r o -R
        char command_r = 'r';
        char command_R = 'R';
        // Se è presente almeno uno dei due
        if (get_value(argumentList, &command_r) != NULL || get_value(argumentList, &command_R) != NULL ) {
            // Prendo l'argomento che l'utente ha inserito con il comando -d
            char *directory = arg->argument;

            // Assegno a DIRNAME la directory che l'utente ha passato
            if ((DIRNAME = (char *) malloc(sizeof(char) * strlen(directory) + 1)) == NULL) {
                perror("ERRORE: Impossibile allocare memoria per DIRNAME.");
                exit(errno);
            }
            strncpy(DIRNAME, directory, strlen(directory) + 1);

            // Rimuovo dalla lista degli argomenti -d
            remove_value(&argumentList, arg);
        } else {
            fprintf(stderr, "ERRORE: impossibile eseguire il comando -d senza eseguire anche un comando -R o -r!\n");
            return;
        }
    }
     
    command = 'f';
    if ((arg = (Argument *) get_value(argumentList, &command)) != NULL) {
        printf("CLIENT: Connessione al socket: \"%s\".\n", arg->argument);
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
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += 10;

    if (openConnection(socketName, TIME, abstime) == -1) {
        perror("ERRORE: Connessione con il server");

        free_arguments();
        printf("\n");
        exit(EXIT_FAILURE);
    }
}

void handle_write_dir(char *optarg) {

    char **argument = NULL;    

    int size = 0;
    char *token = strtok(optarg, ",");
    for (; token && size < 2; size++) {
        argument = realloc(argument, sizeof(char *) * (size + 1));

        int length = strlen(token) + 1;

        if ((argument[size] = (char *) malloc(length)) == NULL) {
            perror("ERRORE: Impossibile allocare memoria.");
            exit(errno);
        }
        strncpy(argument[size], token, length);

        token = strtok(NULL, ",");
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
            fprintf(stderr, "ERRORE: l'argomento passato per parametro deve essere un numero! Digita -h per vedere i comandi disponibili.\n");
            return;
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
        
        if (openFile(token, O_CREATE | O_LOCK) == -1) {
            perror("ERRORE: Richiesta di apertura di un File");
            return;
        }
        if (writeFile(token, NULL) == -1) {
            perror("ERRORE: Richiesta di scrittura di un File");
            return;
        }
        if (closeFile(token) == -1) {
            perror("ERRORE: Richiesta di chiusura di un File");
            return;
        }

        token = strtok(NULL, ",");
    }
}

void handle_read_files(char *optarg) {

    size_t bufferSize;
    void *buffer = NULL;

    char *token = strtok(optarg, ",");
    while (token) {
        
        if (openFile(token, NO_ARG) == -1) {
            perror("ERRORE: Richiesta di apertura di un File");
            return;
        }

        if (readFile(token, &buffer, &bufferSize) == 0) {

            if (DIRNAME != NULL) {                
                char temp[strlen(token) + 1];
                strncpy(temp, token, strlen(token) + 1);
                write_file_directory(DIRNAME, temp, (char *) buffer);
            }

            free(buffer);
        } else {
            perror("ERRORE: Richiesta di lettura di un file");
            return;
        }

        if (closeFile(token) == -1) {
            perror("ERRORE: richiesta di chiusura di un file");
            return;
        }

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

        if (openFile(token, O_LOCK) == -1) {
            perror("ERRORE: Richiesta di apertura di un File");
            return;
        }

        if (removeFile(token) == -1) {
            perror("ERRORE: Richiesta di rimozione di un File");
            return;            
        }

        token = strtok(NULL, ",");
    }
}

void handle_help_commands() {
    printf("-h : stampa la lista di tutte le opzioni accettate dal client e termina immediatamente\n\n");

    printf("-f filename : specifica il nome del socket AF_UNIX a cui connettersi\n\n");

    printf("-w dirname[,n=0] : invia al server i file nella cartella ‘dirname’, ovvero effettua una richiesta\n");
    printf("di scrittura al server per i file. Se la directory ‘dirname’ contiene altre directory, queste vengono visitate\n");
    printf("ricorsivamente fino a quando non si leggono ‘n‘ file; se n=0 (o non è specificato) non c’è un limite superiore al\n");
    printf("numero di file da inviare al server (tuttavia non è detto che il server possa scriverli tutti)\n\n");

    printf("-W file1[,file2] : lista di nomi di file da scrivere nel server separati da ‘,’\n\n");

    printf("-r file1[,file2] : lista di nomi di file da leggere dal server separati da ‘,’\n\n");

    printf("-R n : tale opzione permette di leggere ‘n’ file qualsiasi attualmente memorizzati nel server; se n=0 (o non è specificato)\n");
    printf("allora vengono letti tutti i file presenti nel server\n\n");

    printf("-d dirname : cartella in memoria secondaria dove scrivere i file letti dal server con l’opzione ‘-r’ o ‘-R’.\n");
    printf("L’opzione -d va usata congiuntamente a ‘-r’ o ‘-R’, altrimenti viene generato un errore; Se si utilizzano le\n");
    printf("opzioni ‘-r’ o ‘-R’senza specificare l’opzione ‘-d’ i file letti non vengono memorizzati sul disco\n\n");
    
    printf("-t time : tempo in millisecondi che intercorre tra l’invio di due richieste successive al server (se non\n");    
    printf("specificata si suppone -t 0, cioè non c’è alcun ritardo tra l’invio di due richieste consecutive);\n\n");

    printf("-c file1[,file2] : lista di file da rimuovere dal server se presenti;\n\n");

    printf("-p : abilita le stampe sullo standard output per ogni operazione. Le stampe associate alle varie operazioni\n");
    printf("riportano almeno le seguenti informazioni: tipo di operazione, file di riferimento, esito e dove è rilevante i bytes letti o scritti.\n\n");
}

// ====================

static int cont = 0;

void check_attribute(char *path, int n) {
    
    struct stat info;

    if (stat(path, &info) == -1) { 
        fprintf(stderr, "ERRORE: Impossibile ottenere le informazioni del File!\n");
        return;
    } 

    if (S_ISREG(info.st_mode)) {

        if (openFile(path, O_CREATE | O_LOCK) == -1) {
            perror("ERRORE: Richiesta di apertura di un file");
            return;
        }
        if (writeFile(path, NULL) == -1) {
            perror("ERRORE: Richiesta di scrittura di un file");
            return;
        }
        if (closeFile(path) == -1) {
            perror("ERRORE: Richiesta di chiusura di un file");
            return;
        }

        cont++;
    } else if (S_ISDIR(info.st_mode)) {
        read_directories(path, n);
    }
}    

void read_directories(char *dirName, int n) {

    DIR *directory = NULL;
    if ((directory = opendir(dirName)) == NULL) {
        perror("ERRORE: Apertura della directory");
        return;
    }

    struct dirent* file;

    char buf[STRING_SIZE];
    if (getcwd(buf, STRING_SIZE) == NULL) {
        perror("ERRORE: Impossibile prendere la directory corrente"); 
        return;
    }

    if (chdir(dirName) == -1) {
        perror("ERRORE: Impossibile spostarsi nella directory"); 
        return;
    }

    while ((errno = 0, file = readdir(directory)) != NULL && (n == 0 || cont < n)) {
        if (strcmp("..", file->d_name) == 0 || strcmp(".", file->d_name) == 0) {
            continue;
        }
        check_attribute(file->d_name, n);
    }

    if (errno != 0) {
        perror("ERRORE: Lettura della directory");
        return;
    } else {
        if ((closedir(directory) == -1)) {
            perror("ERRORE: Chiusura della directory");
            return;
        }
    }
    if (chdir(buf) == -1) {
        perror("ERRORE: Impossibile spostarsi nella directory"); 
        return;
    }    
}

