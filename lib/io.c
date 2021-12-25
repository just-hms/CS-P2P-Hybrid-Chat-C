#include "io.h"

time_t get_current_time(){

    char * buf[256];

    return time(NULL);
}

time_t string_to_time(char * time_string){

    time_t rawtime;
    sscanf(time_string, "%ld", rawtime);

    return rawtime;
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
    int read;
    
    time_t start, end; 
    int i, port;

    char * record;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return -1;

    record = NULL;

    while ((read = getline(&line, &len, fp)) != -1) {
        
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

    char * time_string;

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

    free(time_string);
    fclose(fp);
}

void user_end_session(char * username){
    
    FILE * fPtr;
    FILE * fTemp;
    
    char buf[BUF_LEN];
    char newline[BUF_LEN];

    time_t start, end;

    int port;

    fPtr  = fopen(SESSION_FILE, "r");
    fTemp = fopen(TMP_FILE, "w"); 

    if (fPtr == NULL || fTemp == NULL)
        return;


    while ((fgets(buf, BUF_LEN, fPtr)) != NULL){

        if(starts_with(buf, username)){

            replace_n_with_0(buf);

            sscanf(buf, "%s %d %ld %ld", username, &port, &start, &end);

            if(end == -1){

                fprintf(
                    fTemp, 
                    "%s %d %ld %ld\n", 
                    username, 
                    port, 
                    start, 
                    get_current_time()
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

char * user_get_online_list(){
    
    FILE * fp;
    char * line = NULL;
    int len = 0;
    
    time_t start, end;

    int i, port, count;

    char * buf;

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
    buf = malloc((sizeof(char) + 2) * count);
    
    buf[0] = '\0';

    while (getline(&line, &len, fp) != -1) {
        replace_n_with_0(line);

        sscanf(line, "%s %d %ld %ld", username, &port, &start, &end);

        if(end == -1){
            strcat(buf, username);
            strcat(buf, "\n");
        }
    }   

    strcat(buf, "\0");
    
    if (line)
        free(line);

    return buf;
}