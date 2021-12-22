
#include "utils.h"

int receive_message(int i, char * buffer){

    int res;

    res = recv(i, (void*)buffer, BUF_LEN, 0);
    
    if(res < 0){
        buffer = NULL;
        return -1;
    }

    buffer[BUF_LEN - 1] = '\0'; 
    return res;
}

int send_message(int i, char * message){
    
    int len, res;
    char buffer[BUF_LEN];
    
    len = strnlen(message, BUF_LEN) + 1;
    
    if(len >= BUF_LEN)
        return -1;
    
    strcpy(buffer, message);

    buffer[len] = '\0';

    res = send(i, (void*) buffer, BUF_LEN, 0);
    
    return res;
}

/* FIX ME strlen not secure */

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
