#ifndef SERVER_H
#define SERVER_H

#include <http_parser.h>
#include <stddef.h>
#include <stdint.h>

#include "routes.h"

typedef struct Server {
  int32_t fd;
  size_t max_pending_connections;
  size_t max_request_size;
  const char *port;
  struct addrinfo *res;
  uint8_t running;
  Routes *r;
} Server;

Server *createServer(const char *port, const size_t max_pending_connections,
                     const size_t max_request_size, Routes *r);
void freeServer(Server *s);

int32_t startServer(Server *s);
int32_t acceptClientConnection(Server *s);
void closeServer(Server *s);

int isValidPath(const char *path);
const char *getFileExtension(const char *filename);
void sendHttpResponse(int client_fd, const char *content,
                      const char *content_type, int status_code);
void cleanupHttpRequest(HttpRequest *r);

#endif  // !SERVER_H
