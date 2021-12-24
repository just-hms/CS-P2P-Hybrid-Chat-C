#include <malloc.h>
#include <stdio.h>

#include "utils.h"


struct connection_data{
    int sd;
    int port;
    char username[50];
    struct connection_data * next;
} typedef connection_data;


connection_data * connection(int, char *);
char * request(connection_data *, char *, int);
connection_data * find_connection_by_username(char *);
void close_all_connections();
connection_data * find_connection_by_port(int);

/* remove these ???*/

connection_data * add_connection(int, int, char*);
void remove_connection(int);
int count_connections();
