#include "server.h"

#include <errno.h>
#include <netdb.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct Server
{
    int32_t fd;
    size_t max_pending_connections;
    const char *port;
    struct addrinfo *res;
} Server;

Server *createServer(const char *port, const size_t max_pending_connections) {
    Server *server = malloc(sizeof(*server));

    server->fd = 1;
    server->max_pending_connections = max_pending_connections;
    server->port = port;
    server->res = NULL;

    return server;
}

void freeServer(Server *s) {
    if (!s) {
        fprintf(stderr, "error: Trying to free already freed server\n");
        return;
    }

    free(s->res);
}

// rewrite this
char *read_file(FILE *f) {
    if (f == NULL || fseek(f, 0, SEEK_END)) {
        return NULL;
    }

    long length = ftell(f);

    rewind(f);

    if (length == -1 || (unsigned long)length >= SIZE_MAX) {
        return NULL;
    }

    size_t ulength = (size_t)length;

    char *buffer = malloc(ulength + 1);

    if (buffer == NULL || fread(buffer, 1, ulength, f) != ulength) {
        free(buffer);
        return NULL;
    }

    buffer[ulength] = '\0';

    return buffer;
}

int32_t startServer(Server *s) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int addrinfo_result;

    if ((addrinfo_result = getaddrinfo(NULL, s->port, &hints, &s->res)) != 0) {
        fprintf(stderr, "error: Server getaddrinfo error: %s\n", strerror(errno));
        return -1;
    }

    if ((s->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "error: Server socket error: %s\n", strerror(errno));
        return -1;
    }

    int32_t option = 1;
    setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    int32_t bind_result = 0;

    if ((bind_result = bind(s->fd, s->res->ai_addr, s->res->ai_addrlen)) == -1) {
        fprintf(stderr, "error: Server bind error: %s\n", strerror(errno));
        return -1;
    }

    int32_t listen_result = 0;

    if ((listen_result = listen(s->fd, s->max_pending_connections)) == -1) {
        fprintf(stderr, "Server listen error: %s\n", strerror(errno));
        return -1;
    }

    printf("Server listening on port: %s\n", s->port);

    size_t current_client = -1;

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int client_fd;

        if ((client_fd =
                 accept(s->fd, (struct sockaddr *)&client_addr, &client_addrlen)) == -1) {
            fprintf(stderr, "Server accept error: %s\n", strerror(errno));
            return -1;
        }

        // add parsing of the request, to bring appropriate html

        const char *filename = "index.html";

        FILE *fptr = fopen(filename, "r");

        if (!fptr) {
            fprintf(stderr, "error: File does not exist %s\n", filename);
            close(client_fd);
            return -1;
        }

        char *response = read_file(fptr);

        if (response == NULL || strlen(response) == 0) {
            fprintf(stderr, "error: No response read from file\n");
            close(client_fd);
            fclose(fptr);
            return -1;
        }

        fclose(fptr);
        send(client_fd, response, strlen(response), 0);

        printf("Client connected.\n");
        close(client_fd);
    }

    return 0;
}

void closeServer(Server *s) {
    if (s->fd != -1) {
        close(s->fd);
        s->fd = -1;
    }

    if (s->res != NULL) {
        freeaddrinfo(s->res);
        s->res = NULL;
    }
}
