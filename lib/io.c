#include "io.h"

char * user_find(char * username){
    FILE * fp;
    char * line = NULL;
    int len = 0;
    int read;

    int i;

    char * record;

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

        record = (char *) malloc(len * sizeof(char));
        strcpy(record, line);
        
        fclose(fp);
        return record;
    }

    fclose(fp);

    if (line)
        free(line);

    return NULL;
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

    buf = malloc((strlen(username) + strlen(username) + 2) * sizeof(char));
        
    sprintf(buf, "%s|%s", username, password);
    
    if(strcmp(user_record, buf) == 0){
        free(buf);
        free(user_record);
        return 1;
    }
    

    free(buf);
    free(user_record);
    return 0;
}

char * get_chat(char * chat_name){
    
    char * chat;

    chat = malloc(10 * sizeof(char));


    strcpy(chat, "todo");
    return chat;
}