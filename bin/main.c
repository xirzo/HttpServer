#include <stdlib.h>

#include "server.h"

#define PORT "5000"
#define MAX_PENDING_CONNECTIONS 10

int main(int argc, char *argv[]) {
    Server *s = createServer(PORT, MAX_PENDING_CONNECTIONS);

    startServer(s);

    closeServer(s);

    return EXIT_SUCCESS;
}
