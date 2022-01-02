#include "io.h"

time_t get_current_time(){

    char * buf[256];

    return time(NULL);
}

char * user_find(char * username){
    FILE * fp;
    char * line = NULL;
    int len = 0;

    int i;

    fp = fopen(USER_FILE, "r");
    if (fp == NULL)
        return NULL;

    while (getline(&line, &len, fp) != -1) {
        
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

int user_exists(char * username){
    return user_find(username) != NULL;
}

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
    
    FILE * fp;
    char * user_record;
    char * buf;
    
    user_record = user_find(username); 
    
    if(user_record == NULL)
        return 0;

    buf = malloc((strlen(username) + strlen(password) + 4) * sizeof(char));
        
    sprintf(buf, "%s %s\0", username, password);
    
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
    int len = 0;
    
    time_t start, end; 
    int i, port;

    char * record;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return -1;

    record = NULL;

    while (getline(&line, &len, fp) != -1) {
        
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
    
    if (fp == NULL)
        return;

    /* FIX ME*/
    
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

    char * username[USERNAME_LENGTH];

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
            "%s %d %d %ld %ld\n", 
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

    int len;
    
    sprintf(filename, "%s-%s.txt\0", BUFFERED_MESSAGE_PREFIX, receiver);
    
    fp = fopen(filename, "r");

    if(fp == NULL)
        return "";
    
    while (getline(&line, &len, fp) != -1) {

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

    while (getline(&line, &len, fp) != -1) {

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
        
    }

    strcat(buf, "\0");

    fclose(fp);
    fclose(fTemp);


    remove(filename);
    rename(SERVER_TMP_FILE, filename);

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
    /* TODO explain this */
    char count_sting[20];
    char * line = NULL;
    char * buf;
    int found = 0;
    int count = 0;

    int len;
    
    hanging_message * head = NULL;
    hanging_message * new_hanging = NULL;
    hanging_message * c = NULL;
    hanging_message * to_remove = NULL;

    sprintf(filename, "%s-%s.txt\0", BUFFERED_MESSAGE_PREFIX, receiver);
    
    fp = fopen(filename, "r");

    if(fp == NULL)
        return "";
    
    while (getline(&line, &len, fp) != -1) {

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

    /* TODO explain this */
    buf = malloc(count * (USERNAME_LENGTH + 20 + 20 + 5));
    
    c = head;

    while (c){
        strcat(buf, c->username);
        strcat(buf, " ");

        sprintf(count_sting, "%d", c->count);
        strcat(buf, count_sting);
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
    int len = 0;
    
    time_t start, end;

    int i, port, count;

    char * buf;
    char * time_string = NULL;

    /* FIX ME*/

    char username[USERNAME_LENGTH];

    count = 0;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return NULL;

    while (getline(&line, &len, fp) != -1) {
        
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

    while (getline(&line, &len, fp) != -1) {
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

void save_out_time(char * username){
	
	FILE *fd;
    char filename[100];

    sprintf(filename, "%s-%s.txt\0", OUT_PREFIX, username);
    
    fd = fopen(filename, "w");

    if(fd == NULL)
        return;

	fprintf(fd, "%ld", get_current_time());

    if(fd != NULL)
	    fclose(fd); 
}

time_t get_out_time(char * username){
    time_t out_time;

    FILE * fd;
    
    char filename[100];

    sprintf(filename, "%s-%s.txt\0", OUT_PREFIX, username);
    
    fd = fopen(username, "r");

    if(fd == NULL)
        return -1;
    
    fscanf(fd, "%ld", &out_time);

    if(fd != NULL)
	    fclose(fd);

    return out_time;
}

void clear_out_time(char * username){
    char filename[100];

    sprintf(filename, "%s-%s.txt\0", OUT_PREFIX, username);
    
    remove(filename);
}

void user_print_chat(char * receiver, char * sender){
    FILE * fp;
    
    char filename[USERNAME_LENGTH + USERNAME_LENGTH + 20];
    char sender_to_print [USERNAME_LENGTH];
    char message [100];
    char timestamp_string[USERNAME_LENGTH];
    time_t timestamp;
    char * line = NULL;
    int len = 0;
    int i;

    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, receiver, sender);

    fp = fopen(filename, "r");
    if (fp == NULL)
        return;

    while (getline(&line, &len, fp) != -1) {

        sscanf(line, "%[^'|']|%[^'|']|%[^'|']", sender_to_print, message, timestamp_string);        
        printf("%s %s\n", sender_to_print, message);
    }

    fclose(fp);

    if (line)
        free(line);

    return;

}

time_t user_get_buffered_has_read_time(char * sender, char * receiver){
    
    FILE *fd;
    char filename[100];
    time_t t;
    char time_string[20];

    /* to tell the sender that the receiver has read */

    sprintf(filename, "%s-%s-%s.txt\0", BUFFERED_HAS_READ, sender, receiver);
    
    fd = fopen(filename, "r");

    if(fd == NULL)
        return -1;

    fread (time_string, 1, 20, fd);

    sscanf(time_string, "%ld", &t);

	fclose(fd);

    remove(filename);

    return t;
}

/* overwites cause last is more important */

void user_buffer_has_read(char * receiver, char * sender){
    
    FILE *fd;
    char filename[100];

    /* to tell the sender that the receiver has read */

    sprintf(filename, "%s-%s-%s.txt\0", BUFFERED_HAS_READ, sender, receiver);
    
    fd = fopen(filename, "w");

    if(fd == NULL)
        return;

    /* save the name of the receiver and the time at which he's read */

    fprintf(fd, "%ld",get_current_time());

    if(fd != NULL)
	    fclose(fd);

}

void user_buffer_message(char * sender, char * receiver, char * message, time_t t){
    
    FILE *fd;
    char filename[100];

    sprintf(filename, "%s-%s.txt\0", BUFFERED_MESSAGE_PREFIX, receiver);
    
    fd = fopen(filename, "a");

    if(fd == NULL)
        return;

	fprintf(fd, "%s|%s|%ld\n", sender, message, t);

    if(fd != NULL)
	    fclose(fd);
    
}

void user_sent_message(char * sender, char * receiver, char * message, time_t timestamp, int is_receiver_online){
    
    FILE * fp;
    
    char filename[USERNAME_LENGTH + USERNAME_LENGTH + 20];

    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, sender, receiver);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;

    if(is_receiver_online)
        fprintf(fp,"**|%s|%ld\n", message, timestamp);
    else
        fprintf(fp,"*|%s|%ld\n", message, timestamp);

    fclose(fp);

}

void user_received_message(char * receiver, char * sender, char * message, time_t timestamp){
    
    FILE * fp;
    
    char filename[USERNAME_LENGTH + USERNAME_LENGTH + 20];

    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, receiver, sender);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;

    fprintf(fp,"%s%s|%s|%ld\n", sender, " :=", message, timestamp);
    
    fclose(fp);
}

void user_has_read(char * sender, char * receiver, time_t until_when){
    
    FILE * fp;
    FILE * fTemp;
    
    char * line = NULL;
    int len;

    time_t t;
    
    char filename[USERNAME_LENGTH + USERNAME_LENGTH + 20];
    char message[BUF_LEN];
    char time_string[20];
    char has_read[3];

    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, sender, receiver);
    
    fp = fopen(filename, "r");
    fTemp = fopen(CLIENT_TMP_FILE, "w"); 

    if (fp == NULL || fTemp == NULL)
        return;

    while (getline(&line, &len, fp) != -1) {
        
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
    rename(CLIENT_TMP_FILE, filename);

    return;
}

int is_in_contacts(char * owner, char* username){
    
    /* TODO */
    
    return 1;
}