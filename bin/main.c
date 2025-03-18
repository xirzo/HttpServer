#include <stdlib.h>

#include "routes.h"
#include "server.h"

#define PORT "5000"
#define MAX_PENDING_CONNECTIONS 10
#define MAX_NUMBER_OF_ROUTES 10
#define MAX_REQUEST_SIZE 1024 * 1024

int main(int argc, char *argv[]) {
    Routes *r = createRoutes(MAX_NUMBER_OF_ROUTES);

    insertRoute(r, "", "index.html");

    Server *s = createServer(PORT, MAX_PENDING_CONNECTIONS, MAX_REQUEST_SIZE, r);

    startServer(s);

    while (isServerRunning(s)) {
        acceptClientConnection(s);
    }

    closeServer(s);
    freeServer(s);

    return EXIT_SUCCESS;
}
