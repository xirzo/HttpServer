#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <stdint.h>

enum RequestType {
    GET,
};

struct Server {
    struct addrinfo *res;
    int32_t fd;
};

void init_server(struct Server *server);

int32_t start_server(struct Server *server, const char *port,
                     const int32_t max_pending_con);
void close_server(struct Server *server);

#endif  // !SERVER_H
