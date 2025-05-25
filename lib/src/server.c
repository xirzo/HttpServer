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

typedef struct {
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
    {"xlsx",
     "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx",
     "application/"
     "vnd.openxmlformats-officedocument.presentationml.presentation"},
    {"doc", "application/msword"},
    {"docx",
     "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
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

Server *createServer(const char *port, const size_t max_pending_connections,
                     const size_t max_request_size, Routes *r) {
  Server *server = malloc(sizeof(*server));

  if (!server) {
    fprintf(stderr, "error: Failed to allocate memory for server\n");
    return NULL;
  }

  server->fd = -1;
  server->max_pending_connections = max_pending_connections;
  server->max_request_size = max_request_size;
  server->port = port;
  server->res = NULL;
  server->r = r;
  server->running = 0;

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

  if (s->res) {
    freeaddrinfo(s->res);
    s->res = NULL;
  }

  if (s->fd != -1) {
    close(s->fd);
    s->fd = -1;
  }

  free(s);
}

char *read_file(FILE *f) {
  if (f == NULL || fseek(f, 0, SEEK_END)) {
    return NULL;
  }

  if (fseek(f, 0, SEEK_END) != 0) {
    return NULL;
  }

  long length = ftell(f);

  rewind(f);

  if (length < 0 || (unsigned long)length >= SIZE_MAX - 1) {
    return NULL;
  }

  size_t ulength = (size_t)length;

  char *buffer = malloc(ulength + 1);

  if (buffer == NULL) {
    return NULL;
  }

  size_t bytes_read = fread(buffer, 1, ulength, f);

  if (bytes_read != ulength) {
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
    fprintf(stderr, "error: Server getaddrinfo error: %s\n",
            gai_strerror(addrinfo_result));
    return -1;
  }

  if ((s->fd = socket(s->res->ai_family, s->res->ai_socktype,
                      s->res->ai_protocol)) == -1) {
    fprintf(stderr, "error: Server socket error: %s\n", strerror(errno));
    freeaddrinfo(s->res);
    return -1;
  }

  int32_t option = 1;
  if (setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) ==
      -1) {
    fprintf(stderr, "error: Server setsockopt error: %s\n", strerror(errno));
    close(s->fd);
    freeaddrinfo(s->res);
    return -1;
  }

  if (bind(s->fd, s->res->ai_addr, s->res->ai_addrlen) == -1) {
    fprintf(stderr, "error: Server bind error: %s\n", strerror(errno));
    close(s->fd);
    freeaddrinfo(s->res);
    return -1;
  }

  if (listen(s->fd, s->max_pending_connections) == -1) {
    fprintf(stderr, "Server listen error: %s\n", strerror(errno));
    close(s->fd);
    freeaddrinfo(s->res);
    return -1;
  }

  printf("Server listening on port: %s\n", s->port);
  s->running = 1;
  return 0;
}

int32_t acceptClientConnection(Server *s) {
  while (s->running) {  // ✅ Added: Check if server is still running
    struct sockaddr_storage client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int client_fd;

    if ((client_fd = accept(s->fd, (struct sockaddr *)&client_addr,
                            &client_addrlen)) == -1) {
      fprintf(stderr, "error: Server accept error: %s\n", strerror(errno));
      continue;
    }

    char *client_buffer = malloc(sizeof(char) * s->max_request_size);

    if (!client_buffer) {
      fprintf(stderr, "error: Failed to allocate client buffer\n");
      close(client_fd);
      continue;
    }

    ssize_t value_read =
        read(client_fd, client_buffer, s->max_request_size - 1);

    if (value_read <= 0) {
      fprintf(stderr, "error: Client disconnected or read error: %s\n",
              strerror(errno));
      free(client_buffer);
      close(client_fd);
      continue;
    }

    if (value_read >= s->max_request_size - 1) {
      fprintf(stderr, "error: Client data exceeds buffer size. Truncating.\n");
      client_buffer[s->max_request_size - 1] = '\0';
    } else {
      client_buffer[value_read] = '\0';
    }

    HttpRequest *r = NULL;

    initHttpRequest(&r);
    parseRequestLine(r, client_buffer);

    if (!r->uri || !isValidPath(r->uri + 1)) {
      fprintf(stderr, "error: Invalid or unsafe URI path\n");
      sendHttpResponse(client_fd, "400 Bad Request", "text/plain", 400);
      cleanupHttpRequest(r);
      free(client_buffer);
      close(client_fd);
      continue;
    }

    char *host_header = strstr(client_buffer, "Host:");
    char host[256] = {0};

    if (host_header) {
      char *host_start = host_header + 5;
      while (*host_start == ' ' || *host_start == '\t') {
        host_start++;
      }

      int parsed = sscanf(host_start, "%255[^:\r\n ]", host);
      if (parsed == 1) {
        printf("Request for host: %s\n", host);
      }
    }

    char *key = malloc(strlen(r->uri));

    if (!key) {
      fprintf(stderr, "error: Failed to allocate memory for key\n");
      sendHttpResponse(client_fd, "500 Internal Server Error", "text/plain",
                       500);
      cleanupHttpRequest(r);
      free(client_buffer);
      close(client_fd);
      continue;
    }

    strcpy(key, r->uri + 1);

    const char *filename = getRoute(s->r, key);

    if (!filename) {
      fprintf(stderr, "error: Route not found for key: %s\n", key);
      sendHttpResponse(client_fd, "404 Not Found", "text/html", 404);
      free(key);
      cleanupHttpRequest(r);
      free(client_buffer);
      close(client_fd);
      continue;
    }

    FILE *fptr = fopen(filename, "r");
    if (!fptr) {
      fprintf(stderr, "error: File does not exist %s\n", filename);
      sendHttpResponse(client_fd, "404 Not Found", "text/html", 404);
      free(key);
      cleanupHttpRequest(r);
      free(client_buffer);
      close(client_fd);
      continue;
    }

    char *response = read_file(fptr);
    fclose(fptr);

    if (response == NULL || strlen(response) == 0) {
      fprintf(stderr, "error: No response read from file\n");
      sendHttpResponse(client_fd, "500 Internal Server Error", "text/plain",
                       500);
      free(response);
      free(key);
      cleanupHttpRequest(r);
      free(client_buffer);
      close(client_fd);
      continue;
    }

    const char *extension = getFileExtension(filename);
    const char *content_type =
        extension ? getContentType(extension) : "text/plain";

    sendHttpResponse(client_fd, response, content_type, 200);

    printf("Client request processed successfully.\n");

    free(response);
    free(key);
    cleanupHttpRequest(r);
    free(client_buffer);
    close(client_fd);
  }

  return 0;
}

void closeServer(Server *s) {
  s->running = 0;

  if (s->fd != -1) {
    close(s->fd);
    s->fd = -1;
  }

  if (s->res != NULL) {
    freeaddrinfo(s->res);
    s->res = NULL;
  }
}

int isValidPath(const char *path) {
  if (!path) {
    return 0;
  }

  if (strstr(path, "..") != NULL) {
    return 0;
  }

  if (path[0] == '/') {
    return 0;
  }

  return 1;
}

const char *getFileExtension(const char *filename) {
  if (!filename) {
    return NULL;
  }

  const char *dot = strrchr(filename, '.');

  if (!dot || dot == filename) {
    return NULL;
  }

  return dot + 1;
}

void sendHttpResponse(int client_fd, const char *content,
                      const char *content_type, int status_code) {
  if (!content || !content_type) {
    return;
  }

  char *response_header;
  const char *status_text;

  switch (status_code) {
    case 200:
      status_text = "OK";
      break;
    case 404:
      status_text = "Not Found";
      break;
    case 400:
      status_text = "Bad Request";
      break;
    case 500:
      status_text = "Internal Server Error";
      break;
    default:
      status_text = "Unknown";
      break;
  }

  size_t content_length = strlen(content);
  size_t header_size =
      snprintf(NULL, 0,
               "HTTP/1.1 %d %s\r\n"
               "Content-Type: %s\r\n"
               "Content-Length: %zu\r\n"
               "Connection: close\r\n"
               "\r\n",
               status_code, status_text, content_type, content_length) +
      1;

  response_header = malloc(header_size);

  if (!response_header) {
    return;
  }

  snprintf(response_header, header_size,
           "HTTP/1.1 %d %s\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n",
           status_code, status_text, content_type, content_length);

  send(client_fd, response_header, strlen(response_header), 0);
  send(client_fd, content, content_length, 0);

  free(response_header);
}

void cleanupHttpRequest(HttpRequest *r) {
  if (r) {
    freeHttpRequest(r);
  }
}
