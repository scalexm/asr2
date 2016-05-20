#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>

typedef struct iterator iterator_t;

typedef struct client {
    int socket;
    pthread_t thread;
    iterator_t * it;
} client_t;

void * client_thread(void *);

#endif
