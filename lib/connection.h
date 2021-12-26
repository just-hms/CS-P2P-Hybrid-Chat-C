#include <malloc.h>
#include <stdio.h>

#include "utils.h"
#include "endpoint.h"


struct connection_data{
    int sd;
    int port;
    char username[50];
    struct connection_data * next;

} typedef connection_data;

connection_data * connection(int);
void connection_set_master(fd_set *);


char * make_request(connection_data *, char *, int);

/* TODO these */

connection_data * find_connection_by_username(char *);
connection_data * find_connection_by_port(int);
connection_data * find_connection_by_sd(int);

void close_all_connections();