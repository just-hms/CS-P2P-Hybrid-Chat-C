
#include "utils.h"

char base_directory[USERNAME_LENGTH + 20];

char * get_base_directory(){ return base_directory; }

void user_create_folder(char * username){
    struct stat st = {0};
    char filename[100];
    FILE * fp;
    
    if (stat(DATA_FOLDER, &st) == -1) {
        mkdir(DATA_FOLDER, 0755);
    }

    sprintf(base_directory, "%s%s", DATA_FOLDER, username);

    if (stat(base_directory, &st) == -1) {
        mkdir(base_directory, 0755);
    }

    if(strcmp(username, SERVER_NAME) == 0)
        return;

    sprintf(filename, "%s/%s", base_directory, CONTACTS_FILE);

    fp = fopen(filename, "a");

    fclose(fp);
    
}

int receive_message(int sd, char * buf){

    int res;
    size_t message_length;

    res = recv(sd, (void*)&message_length, sizeof(size_t), 0);

    message_length = ntohl(message_length);

    if(res <= 0){    
        buf = NULL;
        return res;
    }

    if(message_length > BUF_LEN){
        buf = NULL;
        return -1;
    }

    res = recv(sd, (void*)buf, message_length, 0);
    
    if(res <= 0){
        buf = NULL;
        return res;
    }

    return res;
}

int send_message(int sd, char * message){
    
    int res;
    char buf[BUF_LEN];

    size_t message_length, message_length_net;

    message_length = strlen(message) + 1;

    message_length_net = htonl(message_length); 
    
    if(message_length >= BUF_LEN)
        return -1;

    res = send(sd, (void*)&message_length_net, sizeof(size_t), 0);

    if(res < 0) 
        return res;
    
    strcpy(buf, message);

    buf[message_length] = '\0';

    res = send(sd, (void*) buf, message_length, 0);
    
    return res;
}

int starts_with(char * first, char * second){
    int len_first = strlen(first);
    int len_second = strlen(second);

    if(len_first > len_second)
        return strncmp(first, second, len_second) == 0;
    
    return strncmp(first, second, len_first) == 0;
}

char * build_string(char * s){
    char * res;
    res = malloc(sizeof(char) * (strlen(s) + 1));
    strcpy(res, s);
    return res;
}

char * replace_n_with_0(char * string){
    int i;
    int len;
    len = strlen(string);
    for (i = 0; i < len; i++){
        if(string[i] == '\n'){
            string[i] = '\0';
            return string;
        }
        if(string[i] == '\0')
            return string;
    }

    return string;
    
}

time_t get_current_time(){  return time(NULL); }

char * time_to_str(time_t t){
    char * buf;
    struct tm tin;
    buf = malloc(20 * sizeof(char));
    memcpy(&tin, localtime(&t), sizeof(struct tm)); 
    sprintf(buf, "%d:%d:%d", tin.tm_hour, tin.tm_min, tin.tm_sec);

    return buf;
}