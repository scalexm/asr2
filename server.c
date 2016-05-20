#include "client.h"
#include "list.h"
#include "server.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>

#define HANDLE_ERROR(str) do { \
    printf("%s: %s\n", str, strerror(errno)); \
    server_kill(); \
 } while(0)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
list_t * clients;
int server_socket = -1;
int in_shutdown = 0;
char * path = "./";
pthread_t main_thread;

void server_kill() {
    printf("shutdown\n");
    pthread_mutex_lock(&mutex);
    in_shutdown = 1;
    for (iterator_t * it = list_begin(clients); it != list_end(clients); it = iterator_next(it)) {
        client_t * c = iterator_deref(it);
#ifndef __APPLE__
        pthread_kill(c->thread, SIGUSR1);
        pthread_join(c->thread, NULL);
#endif
        free(c);
    }
    list_free(clients);
    pthread_mutex_unlock(&mutex);
    close(server_socket);
}

int main(int argc, char ** argv) {
    if (argc < 3)
        return -1;

    main_thread = pthread_self();

    path = argv[1];
    struct stat st;
    if (stat(path, &st) != 0) {
        printf("`%s` does not exist", path);
        return -1;
    } else if (!S_ISDIR(st.st_mode)) {
        printf("`%s` is not a directory", path);
        return -1;
    }

    if (chdir(path) != 0) {
        HANDLE_ERROR("chdir");
        return -1;
    }

    clients = list_new();

    int port = atoi(argv[2]);
    struct sockaddr_in addr;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    int enable = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(server_socket, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
        HANDLE_ERROR("bind");
        return -1;
    }

    if (listen(server_socket, 0) < 0) {
        HANDLE_ERROR("listen");
        return -1;
    }

    fd_set set, active_set;

    FD_ZERO(&active_set);
    FD_SET(server_socket, &active_set);
    FD_SET(STDIN_FILENO, &active_set);

    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        HANDLE_ERROR("sigprocmask");
        return -1;
    }

    struct sigaction sig;
    sig.sa_flags = 0;
    sigemptyset(&sig.sa_mask);
    sig.sa_handler = server_kill;
    if (sigaction(SIGINT, &sig, NULL) == -1) {
        HANDLE_ERROR("sigaction");
        return -1;
    }

    while (1) {
        set = active_set;
        if (select(FD_SETSIZE, &set, NULL, NULL, NULL) < 0) {
            if (errno != EINTR) {
                HANDLE_ERROR("select");
                return -1;
            }
            return 0;
        }

        if (FD_ISSET(STDIN_FILENO, &set)) {
            char buffer[BUFSIZE] = { 0 };
            fgets(buffer, BUFSIZE, stdin);
            if (strcmp(buffer, "exit\n") == 0)
                break;
        } else if (FD_ISSET(server_socket, &set)) {
            client_t * new = malloc(sizeof(client_t));

            new->socket = accept(server_socket, (struct sockaddr *) &client_addr, &addr_size);
            if (new->socket < 0) {
                free(new);
                HANDLE_ERROR("accept");
                return -1;
            }

            pthread_mutex_lock(&mutex);
            list_push_back(clients, new);
            new->it = iterator_prev(list_end(clients));
            pthread_mutex_unlock(&mutex);

            if (pthread_create(&new->thread, NULL, client_thread, (void*)new) < 0) {
                HANDLE_ERROR("pthread_create");
                return -1;
            }

            printf("client connected\n");
        }
    }

    server_kill();
    return 0;
}
