#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define BUF_LEN 1024
#define MAX_PARAMS_LEN 10
#define SERVER_NAME "__server"

int receive_message(int, char *);
int send_message(int, char*);
int starts_with(char *, char *);
char * build_string(char *);
char * replace_n_with_0(char *);

/* testing */

void params_test(char *, char **, int);