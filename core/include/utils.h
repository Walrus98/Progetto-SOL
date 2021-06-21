#ifndef _UTILS_H
#define _UTILS_H

#define NO_ARG 0x00
#define O_LOCK 0x01
#define O_CREATE 0x02

#define STRING_SIZE 1024

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define DEBUG_PRINT(x) printf x

#define DEBUG(message)        \
    if (DEBUG_ENABLE)         \
    {                         \
        printf("DEBUG: ");    \
        DEBUG_PRINT(message); \
    }

int DEBUG_ENABLE;

int isNumber(const char *s, long *n);

#endif