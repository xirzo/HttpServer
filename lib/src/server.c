#include "server.h"

#include <errno.h>
#include <http_parser.h>
#include <netdb.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "routes.h"

typedef struct
{
    const char *extension;
    const char *content_type;
} ContentTypeMapping;

ContentTypeMapping content_type_table[] = {
    {"jar", "application/java-archive"},
    {"x12", "application/EDI-X12"},
    {"edi", "application/EDIFACT"},
    {"js", "application/javascript"},
    {"bin", "application/octet-stream"},
    {"ogg", "application/ogg"},
    {"pdf", "application/pdf"},
    {"xhtml", "application/xhtml+xml"},
    {"swf", "application/x-shockwave-flash"},
    {"json", "application/json"},
    {"jsonld", "application/ld+json"},
    {"xml", "application/xml"},
    {"zip", "application/zip"},
    {"form", "application/x-www-form-urlencoded"},

    {"mp3", "audio/mpeg"},
    {"wma", "audio/x-ms-wma"},
    {"ra", "audio/vnd.rn-realaudio"},
    {"wav", "audio/x-wav"},

    {"gif", "image/gif"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"png", "image/png"},
    {"tiff", "image/tiff"},
    {"ico", "image/vnd.microsoft.icon"},
    {"icon", "image/x-icon"},
    {"djvu", "image/vnd.djvu"},
    {"svg", "image/svg+xml"},

    {"mixed", "multipart/mixed"},
    {"alternative", "multipart/alternative"},
    {"related", "multipart/related"},
    {"form-data", "multipart/form-data"},

    {"css", "text/css"},
    {"csv", "text/csv"},
    {"event-stream", "text/event-stream"},
    {"html", "text/html"},
    {"htm", "text/html"},
    {"js", "text/javascript"},
    {"txt", "text/plain"},
    {"xml", "text/xml"},

    {"mpeg", "video/mpeg"},
    {"mp4", "video/mp4"},
    {"mov", "video/quicktime"},
    {"wmv", "video/x-ms-wmv"},
    {"avi", "video/x-msvideo"},
    {"flv", "video/x-flv"},
    {"webm", "video/webm"},

    {"apk", "application/vnd.android.package-archive"},
    {"odt", "application/vnd.oasis.opendocument.text"},
    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {"odp", "application/vnd.oasis.opendocument.presentation"},
    {"odg", "application/vnd.oasis.opendocument.graphics"},
    {"xls", "application/vnd.ms-excel"},
    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {"doc", "application/msword"},
    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"xul", "application/vnd.mozilla.xul+xml"},

    {NULL, "text/plain"}};

const char *getContentType(const char *extension) {
    size_t i = 0;

    while (content_type_table[i].extension != NULL) {
        if (strcmp(extension, content_type_table[i].extension) == 0) {
            return content_type_table[i].content_type;
        }

        i++;
    }

    return content_type_table[i].content_type;
}

typedef struct Server
{
    int32_t fd;
    size_t max_pending_connections;
    size_t max_request_size;
    const char *port;
    struct addrinfo *res;
    Routes *r;
} Server;

Server *createServer(const char *port, const size_t max_pending_connections,
                     const size_t max_request_size, Routes *r) {
    Server *server = malloc(sizeof(*server));

    server->fd = 1;
    server->max_pending_connections = max_pending_connections;
    server->max_request_size = max_request_size;
    server->port = port;
    server->res = NULL;
    server->r = r;

    return server;
}

void freeServer(Server *s) {
    if (!s) {
        fprintf(stderr, "error: Trying to free already freed server\n");
        return;
    }

    if (s->r) {
        freeRoutes(s->r);
    }

    free(s->res);
}

char *read_file(FILE *f) {
    if (f == NULL || fseek(f, 0, SEEK_END)) {
        return NULL;
    }

    long length = ftell(f);

    rewind(f);

    if (length == -1 || (unsigned long)length >= SIZE_MAX) {
        return NULL;
    }

    size_t ulength = (size_t)length;

    char *buffer = malloc(ulength + 1);

    if (buffer == NULL || fread(buffer, 1, ulength, f) != ulength) {
        free(buffer);
        return NULL;
    }

    buffer[ulength] = '\0';

    return buffer;
}

int32_t startServer(Server *s) {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int addrinfo_result;

    if ((addrinfo_result = getaddrinfo(NULL, s->port, &hints, &s->res)) != 0) {
        fprintf(stderr, "error: Server getaddrinfo error: %s\n", strerror(errno));
        return -1;
    }

    if ((s->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "error: Server socket error: %s\n", strerror(errno));
        return -1;
    }

    int32_t option = 1;
    setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    int32_t bind_result = 0;

    if ((bind_result = bind(s->fd, s->res->ai_addr, s->res->ai_addrlen)) == -1) {
        fprintf(stderr, "error: Server bind error: %s\n", strerror(errno));
        return -1;
    }

    int32_t listen_result = 0;

    if ((listen_result = listen(s->fd, s->max_pending_connections)) == -1) {
        fprintf(stderr, "Server listen error: %s\n", strerror(errno));
        return -1;
    }

    printf("Server listening on port: %s\n", s->port);

    size_t current_client = -1;

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int client_fd;

        if ((client_fd =
                 accept(s->fd, (struct sockaddr *)&client_addr, &client_addrlen)) == -1) {
            fprintf(stderr, "error: Server accept error: %s\n", strerror(errno));
            return -1;
        }

        char *client_buffer = malloc(sizeof(char) * s->max_request_size);

        ssize_t value_read = read(client_fd, client_buffer, s->max_request_size - 1);

        if (value_read <= 0) {
            fprintf(stderr, "error: Client disconnected or read error: %s\n",
                    strerror(errno));
            free(client_buffer);
            return -1;
        }

        if (value_read >= s->max_request_size - 1) {
            fprintf(stderr, "error: Client data exceeds buffer size. Truncating.\n");
            client_buffer[s->max_pending_connections - 1] = '\0';
        }

        HttpRequest *r;
        init_http_request(&r);
        parse_request_line(r, client_buffer);

        char *host_header = strstr(client_buffer, "Host:");
        char host[256] = {0};

        if (host_header) {
            sscanf(host_header + 5, "%255s", host);
            char *port_separator = strchr(host, ':');
            if (port_separator) {
                *port_separator = '\0';
            }

            fprintf(stderr, "Request for host: %s\n", host);
        }

        char *key = malloc(strlen(r->uri) + 1);

        strcpy(key, r->uri + 1);

        const char *filename = getRoute(s->r, key);

        FILE *fptr = fopen(filename, "r");

        if (!fptr) {
            fprintf(stderr, "error: File does not exist %s\n", filename);
            close(client_fd);
            continue;
        }

        char *response = read_file(fptr);

        if (response == NULL || strlen(response) == 0) {
            fprintf(stderr, "error: No response read from file\n");
            close(client_fd);
            fclose(fptr);
            free(client_buffer);
            return -1;
        }

        fclose(fptr);
        send(client_fd, response, strlen(response), 0);

        printf("Client connected.\n");
        close(client_fd);
    }

    return 0;
}

void closeServer(Server *s) {
    if (s->fd != -1) {
        close(s->fd);
        s->fd = -1;
    }

    if (s->res != NULL) {
        freeaddrinfo(s->res);
        s->res = NULL;
    }
}
