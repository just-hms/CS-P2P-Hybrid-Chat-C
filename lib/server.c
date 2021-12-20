#include "utils.h"

int build_listener(int port){

    int listener, addrlen, ret;
    
    struct sockaddr_in my_addr;
    
    listener = socket(AF_INET, SOCK_STREAM, 0);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));

    if(ret < 0)
        return -1;

    listen(listener, 10);

    return listener;
}

/* TODO maybe add an handler after this */
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

void server(int port, void(*__input)(char *), char* (*__get_request)(char*)){
    
    int ret, listener, i, fdmax; 
    fd_set master, read_fds;
    
    
    char * answer;
    char buffer[BUF_LEN];

    listener = build_listener(port);
    
    if(listener < 0){
        perror("Bind non riuscita\n");
        exit(1);
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(listener, &master);
    FD_SET(STDIN_FILENO, &master); 
    
    fdmax = listener;          
    
    while(1){        
        
        read_fds = master;     
        
        /* (,,,,timeout : NULL) */
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);

        for(i = 0; i <= fdmax; i++) {  

            if(!FD_ISSET(i, &read_fds))
                continue;
        
            if(i == listener) { 
                accept_new_connection(&master, &fdmax, listener);
                continue;
            } 

            if(i == STDIN_FILENO){

                memset(buffer, 0, BUF_LEN);                                
                ret = read(STDIN_FILENO, buffer, BUF_LEN);
                
                if(ret < 0){
                    perror("Errore durante la scrittura: \n");
                    exit(1);
                }

                buffer[BUF_LEN - 1] = '\0';            

                __input(buffer);

                continue;
            }

            receive_message(i, buffer);
            
            answer = __get_request(buffer);

            ret = send_message(i, answer); 
            
            /* FIXME */
            free(answer);
            answer = NULL;

            if(ret < 0){
                perror("Errore in fase di comunicazione con il client: \n");
                exit(1);
            }

            /*
             * TODO answer needs to control closed connection in some ways
             * TODO close connection with client onerror 
            */

            if(ret != 0)
                continue;

            close(i);
            FD_CLR(i, &master);

        }
    }

    close(listener);
}
