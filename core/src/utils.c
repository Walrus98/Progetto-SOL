#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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

/* Read "n" bytes from a descriptor */

/** Evita letture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la lettura da fd leggo EOF
 *   \retval size se termina con successo
 */
int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char *)buf;
    while (left > 0) {
        if ((r = read((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        // EOF
        if (r == 0) {
            return 0; 
        }        
        left -= r;
        bufptr += r;
    }

    return size;
}

/** Evita scritture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la scrittura la write ritorna 0
 *   \retval  1   se la scrittura termina con successo
 */
int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char *) buf;
    while (left > 0) {
        if ((r = write((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        // EOF
        if (r == 0) {
            return 0;
        }
        left -= r;
        bufptr += r;
    }
    return 1;
}