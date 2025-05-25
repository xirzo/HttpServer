#include "routes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Routes *createRoutes(const size_t max_number_of_routes) {
  Routes *routes = malloc(sizeof(*routes));

  routes->max_number_of_routes = max_number_of_routes;
  routes->size = 0;
  routes->keys = malloc(sizeof(char *) * max_number_of_routes);
  routes->values = malloc(sizeof(char *) * max_number_of_routes);

  if (!routes->keys || !routes->values) {
    fprintf(stderr,
            "error: Failed to allocate memory for keys/values arrays\n");
    freeRoutes(routes);
    return NULL;
  }

  for (size_t i = 0; i < max_number_of_routes; i++) {
    routes->keys[i] = malloc(sizeof(char) * MAX_STRING_SIZE);
    routes->values[i] = malloc(sizeof(char) * MAX_STRING_SIZE);

    if (!routes->keys[i] || !routes->values[i]) {
      fprintf(stderr, "error: Failed to allocate memory for route %zu\n", i);
      freeRoutes(routes);
      return NULL;
    }

    routes->keys[i][0] = '\0';
    routes->values[i][0] = '\0';
  }

  return routes;
}

void freeRoutes(Routes *r) {
  if (!r) {
    fprintf(stderr, "error: Trying to free already freed routes\n");
    return;
  }

  if (r->keys) {
    for (size_t i = 0; i < r->max_number_of_routes; i++) {
      free(r->keys[i]);
    }

    free(r->keys);
  }

  if (r->values) {
    for (size_t i = 0; i < r->max_number_of_routes; i++) {
      free(r->values[i]);
    }

    free(r->values);
  }
}

int32_t getRoutesIndex(Routes *r, char key[]) {
  for (size_t i = 0; i < r->size; i++) {
    if (strcmp(r->keys[i], key) == 0) {
      return i;
    }
  }

  return -1;
}

void insertRoute(Routes *r, char key[], char value[]) {
  int32_t index = getRoutesIndex(r, key);

  if (index == -1) {
    strcpy(r->keys[r->size], key);
    r->values[r->size] = value;
    r->size++;
  } else {
    r->values[index] = value;
  }
}

char *getRoute(Routes *r, char key[]) {
  int32_t index = getRoutesIndex(r, key);

  if (index == -1) {
    return NULL;
  } else {
    return r->values[index];
  }
}
