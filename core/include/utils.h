#ifndef _UTILS_H
#define _UTILS_H

#define NO_ARG 0x00
#define O_LOCK 0x01
#define O_CREATE 0x02

#define STRING_SIZE 1024

#define DEBUG_PRINT(x) printf x

#define DEBUG(message)        \
    if (DEBUG_ENABLE)         \
    {                         \
        printf("DEBUG: ");    \
        DEBUG_PRINT(message); \
    }

// #define DEBUG(message)          
//     if (DEBUG_ENABLE)           
//     {                           
//         printf(message);       
//     }

// #define CAZZAFREGNO(message, arg) 
//     if (DEBUG_ENABLE)            
//     {                             
//         printf(message, arg);     
//     }

int DEBUG_ENABLE;

int isNumber(const char *s, long *n);

#endif