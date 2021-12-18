#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUF_LEN 1024
#define REQUEST_LEN 4
#define MAX_MESSAGE_LENGTH 10

// TODO check memory free

int build_listener(int ip, int port){
    int listener, addrlen;
    
    struct sockaddr_in my_addr;
    
    listener = socket(AF_INET, SOCK_STREAM, 0);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = ip;
    
    ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));

    if(ret < 0){
        perror("Bind non riuscita\n");
        exit(1);
    }

    listen(listener, 10);

    return listener;
}

void accept_new_connection(fd_set * master, int * fdmax, int listener){
    
    struct sockaddr_in cl_addr;
    int newfd, addrlen;

    addrlen = sizeof(cl_addr);
    newfd = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
    
    FD_SET(newfd, master); 
    
    if(newfd > *fdmax){ 
        *fdmax = newfd; 
    }
}

char * receive_message(int i){
    
    char * buffer;
    int message_length;

    buffer = malloc(BUF_LEN * sizeof(char));

    recv(i, (void*)buffer, BUF_LEN, 0);

    // FIXME
    message_length = (int) *buffer;

    if(message_length > MAX_MESSAGE_LENGTH)
        return -1;
    
    recv(i, (void*)buffer, message_length, 0);
    
    return buffer;

}

int send_message(int i, char * message){
    
    int len, ret;
    
    len = strnlen(message, BUF_LEN) + 1;
    
    if(len >= BUF_LEN)
        return -1;
    
    sprintf(buffer, "%d", len);

    send(i, (void*) buffer, strlen(buffer) + 1, 0);

    // copy the message in the buffer
    strcpy(buffer, message);

    buffer[len] = '\0';

    // send message
    ret = send(i, (void*) buffer, strlen(buffer) + 1, 0);
    
    return ret;
}

void server(int ip, int port, void(*__input)(), char* (*__read)(char*), int(*__answer)(char*)){
    
    int ret, listener, i, fdmax; 
    fd_set master, read_fds;                
    
    listener = build_listener(ip, port);
    
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listener, &master);
    FD_SET(0, &read_fds); 
    
    fdmax = listener;          
    
    while(1){        
        
        read_fds = master;     
        
        // (,,,,timeout : NULL)
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        
        for(i = 0; i <= fdmax; i++) {  

            if(!FD_ISSET(i, &read_fds))
                continue;
        
            if(i == listener) { 
                
                // TODO something to handle new connection
                accept_new_connection(&master, &fdmax, listener);
                continue;
            } 

            // FIXME
            
            if(i == 0){
                __input();
            }

            message = receive_message(i);
            
            params = __read(message);

            answer = __answer(params); 

            ret = send_message(i, answer); 
            
            if(ret < 0){
                perror("Errore in fase di comunicazione con il client: \n");
            }

            // TODO
            // forse anche in altri casi, quando qualcuno chiama quit
            // trovare un modo carino di metterlo in asnwer

            if(ret != 0)
                continue;

            // TODO something to handle closed connection

            close(i);
            FD_CLR(i, &master);

        }
    }

    close(listener);
}
