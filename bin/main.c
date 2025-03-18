#include <stdlib.h>

#include "server.h"

#define PORT "5000"
#define MAX_PENDING_CONNECTIONS 10

int main(int argc, char *argv[]) {
    Server *s = createServer();

    startServer(s, PORT, MAX_PENDING_CONNECTIONS);

    closeServer(s);

    return EXIT_SUCCESS;
}
