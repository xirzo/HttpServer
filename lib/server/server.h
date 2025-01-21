#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

int32_t start_server(const char *port, const int32_t max_pending_con);
void close_server(void);

#endif  // !SERVER_H
