#include "utils.h"
#include "io.h"

void endpoint(int, void(*)(char *, char **, int), char* (*)(char*, char **, int), int);

/* remove these ??? */
int build_listener(int);
void accept_new_connection(fd_set *, int *, int);
void close_connection(int, fd_set *, int);