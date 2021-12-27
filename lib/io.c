#include "io.h"

time_t get_current_time(){

    char * buf[256];

    return time(NULL);
}

char * time_to_string(time_t t){
    char * buf;
    buf = malloc(21 * sizeof(char));
    strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buf;
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
    
    FILE * fPtr;
    FILE * fTemp;
    
    char buf[BUF_LEN];
    char newline[BUF_LEN];

    time_t start, end;

    char * username[50];

    int port;

    fPtr  = fopen(SESSION_FILE, "r");
    fTemp = fopen(TMP_FILE, "w"); 

    if (fPtr == NULL || fTemp == NULL)
        return;


    while ((fgets(buf, BUF_LEN, fPtr)) != NULL){

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

    fclose(fPtr);
    fclose(fTemp);


    remove(SESSION_FILE);
    rename(TMP_FILE, SESSION_FILE);

    return;
}

char * user_show(char * sender, char * receiver){
    /* TODO */
}

char * user_hanging(char * receiver){
    /* TODO */
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

    char username[50];

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
                
                time_string = time_to_string(start);
                sprintf(line, "%s*%s*%d\n", username, time_string, port);
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

    sprintf(filename, "%s-%s\0", OUT_PREFIX, username);
    
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

    sprintf(filename, "%s-%s\0", OUT_PREFIX, username);
    
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

    sprintf(filename, "%s-%s\0", OUT_PREFIX, username);
    
    remove(filename);
}

void user_print_chat(char * receiver, char * sender){
    FILE * fp;
    
    char filename[50 + 50 + 20];
    char sender_to_print [50];
    char message [100];
    char timestamp_string[50];
    time_t timestamp;
    char * line = NULL;
    int len = 0;
    int i;

    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, sender, receiver);

    fp = fopen(filename, "r");
    if (fp == NULL)
        return;

    while (getline(&line, &len, fp) != -1) {
        
        if(!starts_with("*", line))
            continue;

        sscanf(line, "%[^'|']|%[^'|']|%[^'|']", sender_to_print, message, timestamp_string);
        
        printf("%s %s\n", sender_to_print, message);
        
        fclose(fp);
        return line;
    }

    fclose(fp);

    if (line)
        free(line);

    return NULL;

}

void user_buffer_has_read(char * receiver, char * sender){
    /* TODO */
}
void user_buffer_message(char * sender, char * receiver, char * message, time_t t){
    /* TODO */

}

void user_sent_message(char * sender, char * receiver, char * message, time_t timestamp, int is_receiver_online){
    
    FILE * fp;
    
    char filename[50 + 50 + 20];

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
    
    char filename[50 + 50 + 20];

    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, receiver, sender);
    
    fp = fopen(filename, "a");
    
    if (fp == NULL)
        return;

    fprintf(fp,"%s|%s|%ld\n", sender, message, timestamp);
    
    fclose(fp);
}

void user_has_read(char * sender, char * receiver, time_t until_when){
    
    FILE * fPtr;
    FILE * fTemp;
    
    char buf[BUF_LEN];
    char newline[BUF_LEN];

    time_t timestamp;
    
    char filename[50 + 50 + 20];
    /* TODO FIX ME*/
    char message[BUF_LEN];
    char has_read[3];


    sprintf(filename, "%s-%s-%s.txt\0", CHAT_PREFIX, sender, receiver);
    

    fPtr  = fopen(filename, "r");
    fTemp = fopen(TMP_FILE, "w"); 

    if (fPtr == NULL || fTemp == NULL)
        return;


    while ((fgets(buf, BUF_LEN, fPtr)) != NULL){

        if(!starts_with(buf, "*")){
            fputs(buf, fTemp);
            continue;
        }

        sscanf(buf,"%s %s %ld", has_read, message, &timestamp);
        if(strcmp(has_read, "*") ==  0 && timestamp < until_when)
            fprintf(fTemp, "** %s %ld\n", message, &timestamp);
        else
            fputs(buf, fTemp);
    }

    fclose(fPtr);
    fclose(fTemp);

    remove(filename);
    rename(TMP_FILE, filename);

    return;
}

int is_in_contacts(char * owner, char* username){
    
    /* TODO */
    
    return 1;
}