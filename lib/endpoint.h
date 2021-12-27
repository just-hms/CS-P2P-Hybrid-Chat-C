#include "utils.h"

struct connection_data{
    int sd;
    int port;
    char username[50];
    struct connection_data * next;
    int logged;

} typedef connection_data;

connection_data * connection(int);
void endpoint(int, int(*)(char *, char **, int, char *), char* (*)(char*, char **, int, int, char *), void(*) (int), int);
char * make_request(connection_data *, char *, int);

connection_data * find_connection_by_username(char *);
connection_data * find_connection_by_port(int);
connection_data * find_connection_by_sd(int);
void connection_set_username(int sd, char * username);

void close_all_connections();
