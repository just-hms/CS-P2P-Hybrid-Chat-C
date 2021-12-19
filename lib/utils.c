#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

// set message length before sending it

void receive_message(int i, buffer){
    
    int ret;

    ret = recv(i, (void*)buffer, BUF_LEN, 0);


    if(ret < 0){
        buffer = NULL;
        return;
    }

    buffer[BUF_LEN - 1] = '\0';

}

int send_message(int i, char * message){
    
    int len, ret;
    
    len = strnlen(message, BUF_LEN) + 1;
    
    if(len >= BUF_LEN)
        return -1;
    
    strcpy(buffer, message);

    buffer[len] = '\0';

    ret = send(i, (void*) buffer, BUF_LEN, 0);
    
    return ret;
}