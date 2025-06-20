#include <stdio.h>

#include "routes.h"
#include "server.h"

#define PORT "5000"
#define MAX_PENDING_CONNECTIONS 10
#define MAX_NUMBER_OF_ROUTES 10
#define MAX_REQUEST_SIZE 16 * 1024 * 1024

int main(int argc, char *argv[]) {
  Routes *r = createRoutes(MAX_NUMBER_OF_ROUTES);

  insertRoute(r, "", "index.html");
  insertRoute(r, "style.css", "style.css");

  Server *s = createServer(PORT, MAX_PENDING_CONNECTIONS, MAX_REQUEST_SIZE, r);

  if (startServer(s) != 0) {
    fprintf(stderr, "error: Failed to start server\n");
    freeServer(s);
    return 0;
  }

  acceptClientConnection(s);

  closeServer(s);
  freeServer(s);

  return 0;
}
