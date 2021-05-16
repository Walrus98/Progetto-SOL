#ifndef P_THREAD_UTILS_H
#define P_THREAD_UTILS_H

#define WRITE_ERROR(S) {                            \
    fprintf(stderr, S);                             \
    exit(EXIT_FAILURE);                             \
}

#define WRITE_TERROR(S) {                           \
    fprintf(stderr, S);                             \
    pthread_exit((void *) EXIT_FAILURE);            \
}

#define LOCK(l)                                     \
    if (pthread_mutex_lock(l) != 0) {               \
        WRITE_TERROR("ERRORE FATALE lock\n");       \
    }                                               

#define UNLOCK(l)                                   \
    if (pthread_mutex_unlock(l) != 0) {             \
        WRITE_TERROR("ERRORE FATALE unlock\n");     \
    }                                               

#define WAIT(c, l)                                  \
    if (pthread_cond_wait(c, l) != 0) {             \
        WRITE_TERROR("ERRORE FATALE wait\n");       \
    }

#define SIGNAL(c)                                   \
    if (pthread_cond_signal(c) != 0) {              \
        WRITE_TERROR("ERRORE FATALE signal\n");     \
    }

#define JOIN(t)                                     \
    if (pthread_join(t, NULL) != 0) {               \
        WRITE_ERROR("ERRORE FATALE join\n");        \
    }

#endif