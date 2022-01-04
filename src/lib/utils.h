#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdio.h>


#define BUF_LEN 1024
#define MAX_PARAMS_LEN 10
#define SERVER_NAME "server"
#define USERNAME_LENGTH 50

#define USER_FILE "./server/user.txt"
#define SESSION_FILE "./server/sessions.txt"
#define SERVER_TMP_FILE "./server/server-tmp.txt"
#define BUFFERED_MESSAGE_PREFIX "./server/buffered"
#define BUFFERED_HAS_READ_PREFIX "./server/has_read"

#define USER_PREFIX "./"
#define CLIENT_TMP_FILE "client-tmp.txt"
#define OUT_PREFIX "out"
#define CHAT_PREFIX "chat"
#define GROUP_CHAT_PREFIX "group_chat"
#define CONTACTS_FILE "contacts"


char * get_base_directory();
void user_create_folder(char *);

int receive_message(int, char *);
int send_message(int, char*);

int starts_with(char *, char *);
char * build_string(char *);
char * replace_n_with_0(char *);

/* testing */

void params_test(char *, char **, int);