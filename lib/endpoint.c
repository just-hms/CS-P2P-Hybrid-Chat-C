#include "endpoint.h"

int verbose;

void create_command(char * buffer, void(*f)(char *, char **, int)){

}


int build_listener(int port){

    int listener, res;
    
    struct sockaddr_in my_addr;
    
    listener = socket(AF_INET, SOCK_STREAM, 0);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    res = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));

    if(res < 0)
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
    
    if(verbose)
        printf("new fd := %d\n", newfd);

    if(newfd > *fdmax){ 
        *fdmax = newfd; 
    }
}

/* TODO maybe also an extern function*/
void close_connection(int sd, fd_set * master, int corrupted){
    if(!corrupted)
        close(sd);
    FD_CLR(sd, master);
}

void endpoint(int port, void(*__input)(char *, char **, int), char* (*__get_request)(char*, char **, int), int verbose_param){
    
    int res, listener, i, fdmax, params_len; 
    fd_set master, read_fds;

    char * answer, * command;
    char * params[MAX_INPUT_PARAMS];

    char buffer[BUF_LEN];

    verbose = verbose_param;

    listener = build_listener(port);
    
    if(listener < 0){
        if(verbose) 
            printf("error on binding\n");
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
                if(verbose)
                    printf("new connection asked\n");
                accept_new_connection(&master, &fdmax, listener);
                continue;
            } 

            memset(buffer, 0, BUF_LEN);                                
            
            if(i == STDIN_FILENO){
                if(verbose)
                    printf("new input arrived\n");
                
                res = read(STDIN_FILENO, buffer, BUF_LEN);
                
                if(res < 0){
                    if(verbose)
                        printf("error while typing\n");
                    exit(1);
                }

                buffer[BUF_LEN - 1] = '\0';            

                /* get command */
                
                command = strtok(buffer, " ");
                params_len = 0;

                do{
                    params[params_len] = strtok(NULL, " ");

                } while (params[params_len++] || params_len + 1 == MAX_INPUT_PARAMS);

                /* get command */

                __input(command, params, params_len);
                
                continue;
            }

            res = receive_message(i, buffer);

            if(res < 0){
                if(verbose)
                    printf("[%d] connection error while receiving message\n", i);
                exit(1);
            }
            
            if(res == 0){
                if(verbose)
                    printf("[%d] closed connection while receiving message\n", i);
                close_connection(i, &master, 1);
                continue;
            }

            if(verbose)
                printf("[%d] new message received := %s\n", i, buffer);
            
            /* get command */

            command = strtok(buffer, " ");
            params_len = 0;

            do{
                params[params_len] = strtok(NULL, " ");

            } while (params[params_len++] || params_len + 1 == MAX_INPUT_PARAMS);

            /* get command */

            answer = __get_request(command, params, params_len);

            if(answer == NULL)
                continue;

            res = send_message(i, answer); 
            free(answer);

            if(res < 0){
                if(verbose)
                    printf("[%d] connection error while sending message\n", i);
                close_connection(i, &master, 0);
                continue;
            }

            if(res != 0)
                continue;
            
            printf("[%d] closed connection while sending message\n", i);
            close_connection(i, &master, 0);

            /*
             * TODO answer needs to control closed connection in some ways
             * TODO close connection with client onerror 
            */

        }
    }

    close(listener);
}
