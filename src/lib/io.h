#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

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

time_t user_get_BUFFERED_HAS_READ_PREFIX_time(char *, char *);

/* CLIENT */

void user_create_folder(char *);

void save_out_time(char*);
time_t get_out_time(char*);
void clear_out_time(char*);

void user_print_chat(char *);

void user_sent_message(char *, char*, time_t, int);
int is_in_contacts(char*);
void user_received_message(char *, char *, time_t);
void user_has_read(char *, time_t);

void user_print_group_chat(time_t);
void user_sent_group_message(time_t, char *);
void user_receive_group_message(time_t, char *, char *);

