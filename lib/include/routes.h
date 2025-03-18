#ifndef ROUTES_H
#define ROUTES_H

#include <stddef.h>
#include <stdint.h>

typedef struct Routes Routes;

Routes *createRoutes(const size_t max_number_of_routes);
void freeRoutes(Routes *r);

void insertRoute(Routes *r, char key[], char value[]);
char *getRoute(Routes *r, char key[]);

#endif  // ROUTES_H
