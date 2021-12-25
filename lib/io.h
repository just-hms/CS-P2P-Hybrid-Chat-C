#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define USER_FILE "./server/user.txt"
#define SESSION_FILE "./server/sessions.txt"
#define TMP_FILE "./server/session_tmp.txt"

int user_add(char *, char *);
int user_login(char *, char *);
int user_exists(char *);

int user_get_session(char *);
void user_start_session(char *, int);
void user_end_session(char *);
char * user_show(char *, char *);
char * user_hanging(char *);
char * user_get_online_list();


char * get_chat(char *);

