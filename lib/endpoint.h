#include "utils.h"
#include "io.h"

void endpoint(int, int(*)(char *, char **, int), char* (*)(char*, char **, int, int), void(*)(int), int);
void add_new_connection(int);