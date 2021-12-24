#include "io.h"

char * time_to_string(time_t t){
    struct tm *ptm = gmtime(&t);
    char buf;
    
    buf = malloc(256 * sizeof(char));

    strftime(buf, sizeof buf, "%F %T", ptm);
}

time_t string_to_time(char * time_string){
    struct tm tm;
    strptime(time_string, "%F %T", &tm);
    return mktime(&tm);
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

        for (i = 0; i < len; i++){
            if(line[i] == '\n') {
                line[i] = '\0';
                break;
            }
        }
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

    buf = malloc((strlen(username) + strlen(username) + 4) * sizeof(char));
        
    sprintf(buf, "%s|%s\0", username, password);
    
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
    
    char * start[256]; 
    char * end[256]; 
    int i, port;

    char * record;

    fp = fopen(SESSION_FILE, "r");
    if (fp == NULL)
        return -1;

    record = NULL;

    while ((read = getline(&line, &len, fp)) != -1) {
        
        if(!starts_with(username, line))
            continue;

        for (i = 0; i < len; i++){
            if(line[i] == '\n') {
                line[i] = '\0';
                break;
            }
        }

        record = (char *) malloc(len * sizeof(char));
        strcpy(record, line);
    }

    fclose(fp);
    if (line)
        free(line);

    printf("%s\n", record);

    if(record == NULL)
        return -1;
    
    sscanf(record, "%s|%d|%s|%s", username, port, start, end);

    if(strcmp(end, "alive") == 0)
        return port;

    return -1;
}

void user_start_session(char * username, int port){
    FILE * fp;
    time_t t; 
    char * time_string;

    fp = fopen(SESSION_FILE, "a");
    
    if (fp == NULL)
        return;

    /* FIX ME*/
    
    time(&t);
    time_string = time_to_string(t);
    
    fprintf(
        fp,
        "%s|%d|%s|alive\n", 
        username, 
        port,
        time_string
    );

    free(time_string);
    fclose(fp);
}

void user_end_session(char * username){
    
    /* TODO me*/

}



char * get_chat(char * chat_name){
    
    char * chat;

    chat = malloc(10 * sizeof(char));


    strcpy(chat, "todo");
    return chat;
}