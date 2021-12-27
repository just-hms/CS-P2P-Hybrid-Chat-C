#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define USER_FILE "./server/user.txt"
#define SESSION_FILE "./server/sessions.txt"
#define TMP_FILE "./server/session_tmp.txt"

#define OUT_PREFIX "./client/test/out"
#define CHAT_PREFIX "./client/test/chat"

time_t get_current_time();

/* SERVER */

int user_add(char *, char *);
int user_login(char *, char *);
int user_exists(char *);

int user_get_session(char *);
void user_start_session(char *, int);
void user_end_session(char *, time_t);

char * user_show(char *, char *);
char * user_hanging(char *);
char * user_get_online_list(int);

void user_buffer_has_read(char *, char *);
void user_buffer_message(char *, char *, char *, time_t);

/* CLIENT */

void save_out_time(char * );
time_t get_out_time(char * );
void user_print_chat(char *, char *);

void user_sent_message(char *, char *, char*, time_t, int);
int is_in_contacts(char *, char*);
void user_received_message(char *, char *, char *, time_t);
void user_has_read(char *, char *, time_t);

