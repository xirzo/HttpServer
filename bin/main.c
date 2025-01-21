#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "server/server.h"

int main(int argc, char *argv[]) {
    const char PORT[] = "5000";
    const int32_t MAX_PENDING_CONS = 10;

    start_server(PORT, MAX_PENDING_CONS);

    printf("Server listening on port: %s\n", PORT);

    close_server();

    return EXIT_SUCCESS;
}
