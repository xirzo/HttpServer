#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct addrinfo *res;
int32_t server_fd = -1;

int32_t start_server(const char *port, const int32_t max_pending_con) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int addrinfo_result;

    if ((addrinfo_result = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        printf("Server getaddrinfo error: %s\n", strerror(errno));
        return -1;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Server socket error: %s\n", strerror(errno));
        return -1;
    }

    int32_t bind_result = 0;

    if ((bind_result = bind(server_fd, res->ai_addr, res->ai_addrlen)) == -1) {
        printf("Server bind error: %s\n", strerror(errno));
        return -1;
    }

    int32_t listen_result = 0;

    if ((listen_result = listen(server_fd, max_pending_con)) == -1) {
        printf("Server listen error: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

void close_server() {
    close(server_fd);
    freeaddrinfo(res);
}
