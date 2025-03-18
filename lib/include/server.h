#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "routes.h"

typedef struct Server Server;

Server *createServer(const char *port, const size_t max_pending_connections,
                     const size_t max_request_size, Routes *r);
void freeServer(Server *s);

int32_t startServer(Server *s);
void closeServer(Server *s);

#endif  // !SERVER_H
