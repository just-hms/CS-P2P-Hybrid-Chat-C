#include "io.h"

time_t get_current_time(){  return time(NULL); }

char * find(char * username, char * filename){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    int read;

    fp = fopen(filename, "r");

    if (fp == NULL)
        return NULL;

    while ((read = getline(&line, &len, fp)) != EOF) {
        
        if(!starts_with(username, line))
            continue;

        replace_n_with_0(line);

        fclose(fp);
        return line;
    }

    fclose(fp);

    if (line)
        free(line);

    return NULL;
}

char * user_find(char * username){ return find(username, USER_FILE); }

int user_exists(char * username){ return user_find(username) != NULL;}

int user_add(char * username, char * password){
    FILE * fp;
    char * user_record;
    
    user_record = user_find(username); 
    
    if(user_record != NULL)
        return 0;

    free(user_record);

    fp = fopen(USER_FILE, "a");
    
    if (fp == NULL)
        return 0;

    fprintf(fp,"%s %s\n", username, password);

    fclose(fp);

    return 1;
}

int user_login(char * username, char * password){
    
    char * user_record;
    char * buf;
    
    user_record = user_find(username); 
    
    if(user_record == NULL)
        return 0;

    buf = malloc((strlen(username) + strlen(password) + 4) * sizeof(char));
        
    sprintf(buf, "%s %s", username, password);
    
    if(strcmp(user_record, buf) == 0){
        free(buf);
        free(user_record);
        return 1;
    }
    
    free(buf);
    free(user_record);
    return 0;
}

int user_get_session(char * username){
    
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    
    time_t start, end; 
    int port;
    int read;


    char * record;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return -1;

    record = NULL;

    while ((read = getline(&line, &len, fp)) != EOF) {
        
        if(!starts_with(username, line))
            continue;

        replace_n_with_0(line);

        record = (char *) malloc(len * sizeof(char));
        strcpy(record, line);
    }

    fclose(fp);

    if (line)
        free(line);

    if(record == NULL)
        return -1;
    
    sscanf(record, "%s %d %ld %ld", username, &port, &start, &end);

    free(record);

    if(end == -1)
        return port;

    return -1;
}

void user_start_session(char * username, int port){
    
    FILE * fp;

    fp = fopen(SESSION_FILE, "a");
    
    if (fp == NULL){
        printf("erro staring session of {%s}\n", username);
        return;
    }

    fprintf(
        fp,
        "%s %d %ld -1\n", 
        username, 
        port,
        get_current_time()
    );

    fclose(fp);
}

void user_end_session(char * to_remove, time_t t){
    
    FILE * fp;
    FILE * fTemp;
    
    char buf[BUF_LEN];

    time_t start, end;

    char username[USERNAME_LENGTH];

    int port;

    fp  = fopen(SESSION_FILE, "r");
    fTemp = fopen(SERVER_TMP_FILE, "w"); 

    if (fp == NULL || fTemp == NULL)
        return;


    while ((fgets(buf, BUF_LEN, fp)) != NULL){

        if(!starts_with(buf, to_remove)){
            fputs(buf, fTemp);
            continue;
        }
        
        sscanf(buf, "%s %d %ld %ld", username,  &port, &start, &end);

        if(end != -1){
            fputs(buf, fTemp);
            continue;
        }

        fprintf(
            fTemp, 
            "%s %d %ld %ld\n", 
            username, 
            port, 
            start, 
            t
        );
    }

    fclose(fp);
    fclose(fTemp);


    remove(SESSION_FILE);
    rename(SERVER_TMP_FILE, SESSION_FILE);
}

char * user_show(char * receiver, char * sender){

    FILE *fp;
    FILE * fTemp;

    char filename[100];
    char message[100];
    char username[USERNAME_LENGTH];
    char timestamp_string[20];
    char * line = NULL;
    char * buf;
    int count = 0;
    int read;
    int new_file_lines_count = 0;

    size_t len;
    
    sprintf(filename, "%s-%s.txt", BUFFERED_MESSAGE_PREFIX, receiver);
    
    fp = fopen(filename, "r");

    if(fp == NULL)
        return "";
    
    while ((read = getline(&line, &len, fp)) != EOF) {

        replace_n_with_0(line);

        sscanf(line, "%[^'|']|%[^'|']|%[^'|']", username, message, timestamp_string);        
        
        if(strcmp(username, sender) == 0){
            count++;
        }        
    }
    if(count == 0)
        return "";
    
    fTemp = fopen(SERVER_TMP_FILE, "w"); 
    buf = malloc(count * (100));
    
    buf[0] = '\0';

    rewind(fp);

    while ((read = getline(&line, &len, fp)) != EOF) {

        replace_n_with_0(line);

        sscanf(line, "%[^'|']|%[^'|']|%[^'|']", username, message, timestamp_string);        

        /* check if you already found the username */
        
        if(strcmp(username, sender) == 0){
            strcat(buf, message);
            strcat(buf, "\n");
            continue;
        }
        
        fprintf(
            fTemp, 
            "%s|%s|%s\n", 
            username, 
            message, 
            timestamp_string
        );

        new_file_lines_count++;
        
    }

    strcat(buf, "\0");

    fclose(fp);
    fclose(fTemp);


    remove(filename);

    if(new_file_lines_count > 0){
        rename(SERVER_TMP_FILE, filename);
    }else{
        remove(SERVER_TMP_FILE);
    }

    return buf;
}

struct hanging_message{
    char username[USERNAME_LENGTH];
    int count;
    char timestamp[20];
    struct hanging_message * next; 
} typedef hanging_message;


char * user_hanging(char * receiver){

    FILE *fp;
    char filename[100];
    char message[100];
    char username[USERNAME_LENGTH];
    char timestamp_string[20];
    char count_string[20];
    char * line = NULL;
    char * buf;
    int found = 0;
    int count = 0;
    int read;

    size_t len;
    
    hanging_message * head = NULL;
    hanging_message * new_hanging = NULL;
    hanging_message * c = NULL;
    hanging_message * to_remove = NULL;

    sprintf(filename, "%s-%s.txt", BUFFERED_MESSAGE_PREFIX, receiver);
    
    fp = fopen(filename, "r");

    if(fp == NULL)
        return "";
    
    while ((read = getline(&line, &len, fp)) != EOF) {

        replace_n_with_0(line);

        sscanf(line, "%[^'|']|%[^'|']|%[^'|']", username, message, timestamp_string);        

        /* check if you already found the username */
        
        c = head;

        while (c != NULL){

            if(strcmp(c->username, username) == 0){
                strcpy(c->timestamp, timestamp_string);
                c->count++;
                found = 1;
                break;
            }
            c = c->next;
        }
        if(found){
            found = 0;
            continue;
        }

        /* if not insert it */

        count++;

        new_hanging = malloc(sizeof(hanging_message));
        strcpy(new_hanging->username, username);
        new_hanging->count = 1;
        strcpy(new_hanging->timestamp, timestamp_string);

        if(head == NULL){
            head = new_hanging;
            head->next = NULL;
            continue;
        }
        new_hanging->next = head;
        head = new_hanging;
        
    }

    buf = malloc(count * (USERNAME_LENGTH + 20 + 20 + 5));
    
    buf[0] = '\0';
    
    c = head;

    while (c){
        strcat(buf, c->username);
        strcat(buf, " ");

        sprintf(count_string, "%d", c->count);
        strcat(buf, count_string);
        strcat(buf, " ");
        
        strcat(buf, c->timestamp);
        strcat(buf, "\n");
        to_remove = c;
        c = c->next;
        free(to_remove);
    }
    
    strcat(buf, "\0");
    return buf;
}

char * user_get_online_list(int timestamp_and_port){
    
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    
    time_t start, end;
    int read;

    int port, count;

    char * buf;
    char * time_string = NULL;

    char username[USERNAME_LENGTH];

    count = 0;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return NULL;

    while ((read = getline(&line, &len, fp)) != EOF) {
        
        replace_n_with_0(line);

        sscanf(line, "%s %d %ld %ld", username, &port, &start, &end);
        
        if(end == -1)
            count++;        
    }

    if(count == 0)
        return NULL;

    rewind(fp);
    
    if(timestamp_and_port)
        buf = malloc((sizeof(char) + 20 + 4 + 5) * count);
    else
        buf = malloc((sizeof(char) + 2) * count);
    
    buf[0] = '\0';

    while ((read = getline(&line, &len, fp)) != EOF) {
        replace_n_with_0(line);

        sscanf(line, "%s %d %ld %ld", username,  &port, &start, &end);

        if(end == -1){

            if(timestamp_and_port){

                sprintf(line, "%s*%ld*%d\n", username, start, port);
                strcat(buf, line);
                free(time_string);

            }else{
                
                strcat(buf, username);
                strcat(buf, "\n");
            }
        }
    }   

    strcat(buf, "\0");
    
    if (line)
        free(line);

    return buf;
}

void save_out_time(char* username){
	
	FILE *fp;
    char filename[100];

    sprintf(filename, "%s-%s.txt",OUT_PREFIX, username);
    
    fp = fopen(filename, "w");

    if(fp == NULL) return;

	fprintf(fp, "%ld", get_current_time());

    if(fp != NULL) fclose(fp); 
}

time_t get_out_time(char* username){
    time_t out_time;

    FILE * fp;
    
    char filename[100];

    sprintf(filename, "%s-%s.txt",OUT_PREFIX, username);
    
    fp = fopen(filename, "r");

    if(fp == NULL)
        return -1;
    
    fscanf(fp, "%ld", &out_time);

    if(fp != NULL)
	    fclose(fp);

    return out_time;
}

void clear_out_time(char* username){
    
    char filename[100];

    sprintf(filename, "%s-%s.txt",OUT_PREFIX, username);
        
    remove(filename);
}

void user_print_chat(char * sender){
    FILE * fp;
    
    char filename[20 + USERNAME_LENGTH + USERNAME_LENGTH + 20];
    char sender_to_print [USERNAME_LENGTH];
    char message [100];
    char timestamp_string[USERNAME_LENGTH];
    char * line = NULL;
    size_t len = 0;
    int read;


    sprintf(filename, "%s/%s-%s.txt", get_base_directory(), CHAT_PREFIX, sender);

    fp = fopen(filename, "r");
    if (fp == NULL)
        return;

    while ((read = getline(&line, &len, fp)) != EOF) {

        sscanf(line, "%[^'|']|%[^'|']|%[^'|']", sender_to_print, message, timestamp_string);        
        printf("%s %s\n", sender_to_print, message);
    }

    fclose(fp);

    if (line)
        free(line);

    return;

}

time_t user_get_buffered_has_read_time(char * sender, char * receiver){
    
    FILE *fp;
    char filename[200];
    time_t t;
    char time_string[20];

    /* to tell the sender that the receiver has read */

    sprintf(filename, "%s-%s-%s.txt", BUFFERED_HAS_READ_PREFIX, sender, receiver);
    
    fp = fopen(filename, "r");

    if(fp == NULL)
        return -1;

    fread (time_string, 1, 20, fp);

    sscanf(time_string, "%ld", &t);

	fclose(fp);

    remove(filename);

    return t;
}

void user_buffer_has_read(char * receiver, char * sender){
    
    FILE *fp;
    char filename[100];

    /* to tell the sender that the receiver has read */

    sprintf(filename, "%s-%s-%s.txt", BUFFERED_HAS_READ_PREFIX, sender, receiver);
    
    fp = fopen(filename, "w");

    if(fp == NULL)
        return;

    /* save the name of the receiver and the time at which he's read */

    fprintf(fp, "%ld",get_current_time());

    if(fp != NULL)
	    fclose(fp);

}

void user_buffer_message(char * sender, char * receiver, char * message, time_t t){
    
    FILE *fp;
    char filename[100];

    sprintf(filename, "%s-%s.txt", BUFFERED_MESSAGE_PREFIX, receiver);
    
    fp = fopen(filename, "a");

    if(fp == NULL) return;

	fprintf(fp, "%s|%s|%ld\n", sender, message, t);

    if(fp != NULL)
	    fclose(fp);
    
}

void user_sent_message(char * receiver, char * message, time_t timestamp, int is_receiver_online){
    
    FILE * fp;
    
    char filename[20 + USERNAME_LENGTH + USERNAME_LENGTH + 20];

    sprintf(filename, "%s/%s-%s.txt", get_base_directory(), CHAT_PREFIX, receiver);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;

    if(is_receiver_online)
        fprintf(fp,"**|%s|%ld\n", message, timestamp);
    else
        fprintf(fp,"*|%s|%ld\n", message, timestamp);

    fclose(fp);

}

void user_received_message(char * sender, char * message, time_t timestamp){
    
    FILE * fp;
    
    char filename[20 + USERNAME_LENGTH + USERNAME_LENGTH + 20];

    sprintf(filename, "%s/%s-%s.txt", get_base_directory(), CHAT_PREFIX, sender);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;

    fprintf(fp,"%s%s|%s|%ld\n", sender, " :=", message, timestamp);
    
    fclose(fp);
}

void user_has_read(char * receiver, time_t until_when){
    
    FILE * fp;
    FILE * fTemp;
    
    char * line = NULL;
    size_t len;
    int read;


    time_t t;
    
    char filename[20 + USERNAME_LENGTH + USERNAME_LENGTH + 20];
    char tmp_filname[20 + USERNAME_LENGTH + USERNAME_LENGTH + 20];
    char message[BUF_LEN];
    char time_string[20];
    char has_read[3];

    sprintf(filename, "%s/%s-%s.txt", get_base_directory(), CHAT_PREFIX, receiver);
    sprintf(tmp_filname, "%s/%s.txt", get_base_directory(), CLIENT_TMP_FILE);
    
    fp = fopen(filename, "r");
    fTemp = fopen(tmp_filname, "w"); 

    if (fp == NULL || fTemp == NULL)
        return;

    while ((read = getline(&line, &len, fp)) != EOF) {
        
        if(!starts_with(line, "*")){
            fputs(line, fTemp);
            continue;
        }

        sscanf(line,"%[^'|']|%[^'|']|%[^'|']" , has_read, message, time_string);

        sscanf(time_string, "%ld", &t);

        if(t < until_when){
            fprintf(fTemp, "**|%s|%ld\n", message, t);
            continue;
        }

        fputs(line, fTemp);
    }

    fclose(fp);
    fclose(fTemp);

    remove(filename); 
    rename(tmp_filname, filename);

    return;
}

int is_in_contacts(char* username){
    char filename[100];

    sprintf(filename, "%s/%s", get_base_directory(), CONTACTS_FILE);

    return find(username, filename) != NULL;
}

void user_print_group_chat(time_t group_id){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    int read;
    
    
    char filename[USERNAME_LENGTH + 20 + 20 + 5];

    sprintf(filename, "%s/%s-%ld", get_base_directory(), GROUP_CHAT_PREFIX, group_id);
    
    fp = fopen(filename, "r");
    
    if (fp == NULL)
        return;

    while ((read = getline(&line, &len, fp)) != EOF) {
        printf("%s", line);
    }

    fclose(fp);

    if (line)
        free(line);

    return;
}

void user_sent_group_message(time_t group_id, char * message){
    FILE * fp;
    
    char filename[USERNAME_LENGTH + 20 + 5];

    sprintf(filename, "%s/%s-%ld", get_base_directory(), GROUP_CHAT_PREFIX, group_id);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;
    
    fprintf(fp, "%s\n", message);

    fclose(fp);

}

void user_receive_group_message(time_t group_id, char * sender, char * message){
    FILE * fp;
    
    char filename[USERNAME_LENGTH + 20 + 5];

    sprintf(filename, "%s/%s-%ld", get_base_directory(), GROUP_CHAT_PREFIX, group_id);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;
    
    fprintf(fp, "%s := %s\n", sender, message);

    fclose(fp);
}