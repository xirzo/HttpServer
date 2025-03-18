#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdint.h>

enum RequestType
{
    GET,
};

typedef struct Server Server;

Server *createServer(const char *port, const size_t max_pending_connections);
void freeServer(Server *s);

int32_t startServer(Server *s);
void closeServer(Server *s);

#endif  // !SERVER_H
