#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define USER_FILE "./server/user.txt"
#define SESSION_FILE "./server/sessions.txt"

int user_add(char *, char *);
int user_login(char *, char *);
int user_exists(char *);

int user_get_session(char *);
void user_start_session(char *, int);
void user_end_session(char *);

char * get_chat(char *);

