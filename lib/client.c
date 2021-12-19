#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

void client(char* server_ip, int server_port, char* (*__make_request)(), void (*__handle_response)(char* )){
    
    int ret, sd;
    struct sockaddr_in srv_addr;
    char buffer[BUF_LEN];
    char * request;
    
    sd = socket(AF_INET,SOCK_STREAM,0);
    
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = server_port;
    inet_pton(AF_INET, server_ip, &srv_addr.sin_addr);
    
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(-1);
    }

    while(1){

        request = __make_request(sd);

        ret = send_message(request);

        if(ret < 0){
            perror("Errore in fase di invio comando: \n");
            exit(-1);
        }
        
        receive_message(sd, buffer);

        if(buffer == NULL){
            perror("Errore in fase di ricezione: \n");
            exit(1);
        }

        __handle_response(buffer);
    }

    close(sd);
}