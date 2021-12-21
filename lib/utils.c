
#include "utils.h"

int receive_message(int i, char * buffer){

    int ret;

    ret = recv(i, (void*)buffer, BUF_LEN, 0);
    
    if(ret < 0){
        buffer = NULL;
        return -1;
    }

    buffer[BUF_LEN - 1] = '\0'; 
    return ret;
}

int send_message(int i, char * message){
    
    int len, ret;
    char buffer[BUF_LEN];
    
    len = strnlen(message, BUF_LEN) + 1;
    
    if(len >= BUF_LEN)
        return -1;
    
    strcpy(buffer, message);

    buffer[len] = '\0';

    ret = send(i, (void*) buffer, BUF_LEN, 0);
    
    return ret;
}

// FIX ME strlen not secure

int starts_with(char * first, char * second){
    int length_first = strlen(first);
    int length_second = strlen(second);

    if(length_first > length_second)
        return strncmp(first, second, length_first) == 0;
    
    return strncmp(first, second, length_second) == 0;
}