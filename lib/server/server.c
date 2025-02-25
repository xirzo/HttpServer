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

void init_server(struct Server *server) {
    server->res = NULL;
    server->fd = 1;
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

int32_t start_server(struct Server *server, const char *port,
                     const int32_t max_pending_con) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int addrinfo_result;

    if ((addrinfo_result = getaddrinfo(NULL, port, &hints, &server->res)) != 0) {
        printf("Server getaddrinfo error: %s\n", strerror(errno));
        return -1;
    }

    if ((server->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Server socket error: %s\n", strerror(errno));
        return -1;
    }

    int32_t option = 1;
    setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    int32_t bind_result = 0;

    if ((bind_result = bind(server->fd, server->res->ai_addr, server->res->ai_addrlen)) ==
        -1) {
        printf("Server bind error: %s\n", strerror(errno));
        return -1;
    }

    int32_t listen_result = 0;

    if ((listen_result = listen(server->fd, max_pending_con)) == -1) {
        printf("Server listen error: %s\n", strerror(errno));
        return -1;
    }

    printf("Server listening on port: %s\n", port);

    size_t current_client = -1;

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int client_fd;

        if ((client_fd = accept(server->fd, (struct sockaddr *)&client_addr,
                                &client_addrlen)) == -1) {
            printf("Server accept error: %s\n", strerror(errno));
            return -1;
        }

        // add parsing of the request, to bring appropriate html

        FILE *fptr = fopen("index.html", "r");

        char *response = read_file(fptr);

        fclose(fptr);

        send(client_fd, response, strlen(response), 0);

        printf("Client connected.\n");
        close(client_fd);
    }

    return 0;
}

void close_server(struct Server *server) {
    if (server->fd != -1) {
        close(server->fd);
        server->fd = -1;
    }

    if (server->res != NULL) {
        freeaddrinfo(server->res);
        server->res = NULL;
    }
}
