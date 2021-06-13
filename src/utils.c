#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/utils.h"

int isNumber(const char *arg, long *num) {

    char *string = NULL;

    long valore = strtol(arg, &string, 10);

    if (errno == ERANGE) {
        perror("ERRORE: overflow");
        exit(errno);
    }

    if (string != NULL && *string == (char) 0) {
        *num = valore;
        return 1;   
    }

    return 0;
}
