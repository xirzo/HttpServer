#include <stdlib.h>

#include "server/server.h"

int main(int argc, char *argv[]) {
    const char PORT[] = "5000";
    const int32_t MAX_PENDING_CONS = 10;

    struct Server *server = malloc(sizeof(struct Server));

    server->fd = 1;
    server->res = NULL;

    start_server(server, PORT, MAX_PENDING_CONS);

    close_server(server);

    return EXIT_SUCCESS;
}
