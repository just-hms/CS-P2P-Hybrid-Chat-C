#include <malloc.h>
#include <stdio.h>

/* TODO add ip ??? */

struct connection{
    int sd;
    int port;
    /* username */
    struct connection * next;
} typedef connection;

void add_connection(int, int);

void remove_connection(int);

connection * find_connection(int);

int count_connections();