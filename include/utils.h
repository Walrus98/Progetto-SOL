#ifndef _UTILS_H
#define _UTILS_H

#define DEBUG(message)   \
    if (DEBUG_ENABLE)    \
    {                    \
        printf(message); \
    }

int DEBUG_ENABLE;

int isNumber(const char *s, long *n);

#endif