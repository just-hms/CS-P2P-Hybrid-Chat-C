#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE     1024
#define RESPONSE_LEN    9

void client(char* server_ip, int server_port, char* (*__request)(), void (*__response)(char* )){
    
    int ret, sd;
    struct sockaddr_in srv_addr;
    char buffer[BUFFER_SIZE];
    char * request;
    
    /* Creazione socket */
    sd = socket(AF_INET,SOCK_STREAM,0);
    
    /* Creazione indirizzo del server */
    memset(&srv_addr, 0, sizeof(srv_addr)); // Pulizia 
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = server_port;
    inet_pton(AF_INET, server_ip, &srv_addr.sin_addr);
    
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    if(ret < 0){
        perror("Errore in fase di connessione: \n");
        exit(-1);
    }

    while(1){

        // TODO build something with a dimension
        request = __request(); 
        
        // TODO copy request in the buffer or something similiar

        ret = send(sd, request, strlen(request), 0);
        if(ret < 0){
            perror("Errore in fase di invio comando: \n");
            exit(-1);
        }
        
        // Attendo risposta
        ret = recv(sd, (void*)buffer, RESPONSE_LEN, 0);
        
        if(ret < 0){
            perror("Errore in fase di ricezione: \n");
            exit(-1);
        }

        __response(buffer);
    }
    close(sd);
}

void kek(){
    printf("keke");
}