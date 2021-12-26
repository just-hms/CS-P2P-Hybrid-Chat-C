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
    int read;

    int i;

    fp = fopen(USER_FILE, "r");
    if (fp == NULL)
        return NULL;

    while ((read = getline(&line, &len, fp)) != -1) {
        
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

    fprintf(fp,"%s|%s\n", username, password);

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
    int i, port, sd;

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
    
    sscanf(record, "%s %d %d %ld %ld", username, &sd, &port, &start, &end);

    free(record);

    if(end == -1)
        return port;

    return -1;
}

char * user_get_username_by_sd(int sd_to_find){
    
    FILE * fp;
    char * line = NULL;
    int len = 0;
    
    time_t start, end; 
    int i, port, sd;

    char username[50];
    char * username_found = NULL;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return NULL;

    while (getline(&line, &len, fp) != -1) {
        
        replace_n_with_0(line);

        sscanf(line, "%s %d %d %ld %ld", username, &sd,  &port, &start, &end);

        if(sd == sd_to_find){
            
            if(username_found == NULL)
                username_found = malloc(50 * sizeof(char));
            
            strcpy(username_found, username);
            continue;
        }
    }

    fclose(fp);

    if (line)
        free(line);

    return username_found;
}

void user_start_session(char * username, int port, int sd){
    
    FILE * fp;

    fp = fopen(SESSION_FILE, "a");
    
    if (fp == NULL)
        return;

    /* FIX ME*/
    
    fprintf(
        fp,
        "%s %d %d %ld -1\n", 
        username, 
        sd,
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

    int port, sd;

    fPtr  = fopen(SESSION_FILE, "r");
    fTemp = fopen(TMP_FILE, "w"); 

    if (fPtr == NULL || fTemp == NULL)
        return;


    while ((fgets(buf, BUF_LEN, fPtr)) != NULL){

        if(starts_with(buf, to_remove)){

            sscanf(buf, "%s %d %d %ld %ld", username, &sd,  &port, &start, &end);

            if(end == -1){

                fprintf(
                    fTemp, 
                    "%s %d %d %ld %ld\n", 
                    username, 
                    sd,
                    port, 
                    start, 
                    t
                );

                continue;
            }
        }

        fputs(buf, fTemp);
    }

    fclose(fPtr);
    fclose(fTemp);


    remove(SESSION_FILE);
    rename(TMP_FILE, SESSION_FILE);

    return;
}

char * get_chat(char * chat_name){
    
    /* TODO */
    
    char * chat;

    chat = malloc(10 * sizeof(char));

    strcpy(chat, "todo");
    return chat;
}

char * user_show(char * sender, char * receiver){
    /* TODO 
        flush buffered messages
    */
}

char * user_hanging(char * receiver){
    /* TODO */
}

char * user_get_online_list(int timestamp_and_port){
    
    FILE * fp;
    char * line = NULL;
    int len = 0;
    
    time_t start, end;

    int i, port, count, sd;

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

        sscanf(line, "%s %d %d %ld %ld", username, &sd, &port, &start, &end);
        
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

        sscanf(line, "%s %d %d %ld %ld", username, &sd,  &port, &start, &end);

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
    
    fd = fopen(username, "w");

    if(fd == NULL)
        return;

	fprintf(fd, "%ld", get_current_time());

    if(fd != NULL)
	    fclose(fd); 
}

time_t get_out_time(char * username){
    time_t out_time;

    FILE * fd;
    
    fd = fopen(username, "r");

    if(fd == NULL)
        return -1;
    
    fscanf(fd, "%ld", &out_time);

    if(fd != NULL)
	    fclose(fd);

    return out_time;
}

void clear_out_time(char * username){
    remove(username);
}
