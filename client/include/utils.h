#ifndef _UTILS_H
#define _UTILS_H

int DEBUG_ENABLE;

#define DEBUG(message)   \
    if (DEBUG_ENABLE)    \
    {                    \
        printf(message); \
    }

int isNumber(const char *s, long *n);

#endif