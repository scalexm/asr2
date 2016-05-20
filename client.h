#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>

struct iterator;

typedef struct client {
    int socket;
    pthread_t thread;
    struct iterator * it;
} client_t;

void * client_thread(void *);

#endif
