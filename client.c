#include "client.h"
#include "server.h"
#include "list.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>

// cannot use signalfd on Mac OS
#ifndef __APPLE__
#include <sys/signalfd.h>
#endif

#define HANDLE_ERROR(str) do { \
    printf("%s: %s\n", str, strerror(errno)); \
 } while(0)

extern int in_shutdown;
extern pthread_mutex_t mutex;
extern list_t * clients;

typedef struct command {
    char * cmd;
    int (* function) (client_t *, char * arg);
} command_t;

int command_quit(client_t * c, char * arg) {
    return 1;
}

int command_clients(client_t * c, char * arg) {
    size_t size = list_size(clients);
    char buffer[BUFSIZE];
    if (size != 1)
        snprintf(buffer, BUFSIZE, "%zu clients connected\n", size);
    else
        snprintf(buffer, BUFSIZE, "1 client connected\n");
    write(c->socket, buffer, strlen(buffer));
    return 0;
}

int command_size(client_t * c, char * arg) {
    char buffer[BUFSIZE];
    if (arg == NULL)
        snprintf(buffer, BUFSIZE, "`size` needs an argument\n");
    else {
        struct stat st;
        if (stat(arg, &st) == 0) {
            if (S_ISDIR(sb.st_mode))
                snprintf(buffer, BUFFSIZE, "requested file is a directory");
            else
                snprintf(buffer, BUFSIZE, "file weight: %zio\n", st.st_size);
        } else
            snprintf(buffer, BUFSIZE, "cannot find requested file\n");
    }

    write(c->socket, buffer, strlen(buffer));
    return 0;
}

int command_get(client_t * c, char * arg) {
    return 0;
}

int command_list(client_t * c, char * arg) {
    return 0;
}

static command_t commands[] = {
    { "quit", command_quit },
    { "clients", command_clients },
    { "size", command_size },
    { "get", command_get },
    { "list", command_list },
    { NULL, NULL }
};

int parse_input(client_t * c, char * input) {
    char * line_ptr, * space_ptr;
    char * token = strtok_r(input, "\n", &line_ptr);

    while (token != NULL) {
        char * inner_token = strtok_r(token, " ", &space_ptr);
        if (inner_token == NULL)
            continue;

        for (command_t * x = commands; x->cmd; ++x) {
            if (strcmp(inner_token, x->cmd) == 0) {
                inner_token = strtok_r(NULL, " ", &space_ptr);
                int ret = (x->function)(c, inner_token);
                if (ret == 1)
                    return 1;
                break;
            }
        }

        token = strtok_r(NULL, "\n", &line_ptr);
    }
    return 0;
}

void * client_thread(void * data) {
    client_t * c = (client_t *) data;
    char * buffer = NULL;
    ssize_t cur_len = 0;

#ifndef __APPLE__
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);

    int sfd = signalfd(-1, &mask, 0);
    if (sfd < 0) {
        HANDLE_ERROR("signalfd");
        goto quit;
    }

    fd_set set, active_set;

    FD_ZERO(&active_set);
    FD_SET(c->socket, &active_set);
    FD_SET(sfd, &active_set);
#endif

    while (1) {
#ifndef __APPLE__
        set = active_set;
        if (select(FD_SETSIZE, &set, NULL, NULL, NULL) < 0) {
            HANDLE_ERROR("select");
            goto quit;
        }

        if (FD_ISSET(sfd, &set))
            goto quit;
#endif

        char local_buffer[BUFSIZE] = { 0 };
        ssize_t nbytes = read(c->socket, local_buffer, BUFSIZE);

        if (nbytes <= 0)
            goto quit;

        if (buffer == NULL)
            buffer = malloc(nbytes);
        else
            buffer = realloc(buffer, cur_len + nbytes);
        memcpy(&buffer[cur_len], local_buffer, nbytes);
        cur_len += nbytes;

        // searching for a new line character, starting from end
        ssize_t i;
        for (i = cur_len - 1; i >= 0; --i) {
            if (buffer[i] == '\n')
                break;
        }

        if (i >= 0) {
            // ensure that we have a null terminated string
            char * input = malloc(i + 1);
            memcpy(input, buffer, i); // do not include the '\n'
            input[i] = '\0';
            int ret = parse_input(c, input);
            free(input);

            if (ret == 1)
                goto quit;

            // removing the part of the message which has just been handled
            if (i == cur_len - 1) {
                free(buffer);
                buffer = NULL;
                cur_len = 0;
            } else {
                cur_len -= (i + 1);
                char * new_buffer = malloc(cur_len);
                memcpy(new_buffer, &buffer[i + 1], cur_len);
                free(buffer);
                buffer = new_buffer;
            }
        }
    }

quit:
    close(c->socket);
    free(buffer);

    if (in_shutdown == 0) { // do not erase client during shutdown
        pthread_mutex_lock(&mutex);
        if (in_shutdown == 0) { // double check (so as to avoid using an atomic flag)
            list_erase(clients, c->it);
            free(c);
            printf("client disconnected\n");
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
