#include "endpoint.h"

int verbose;

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

void accept_new_connection(fd_set * master, int * fdmax, int listener){
    
    struct sockaddr_in cl_addr;
    int newfd, addrlen;

    addrlen = sizeof(cl_addr);
    
    newfd = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
    
    FD_SET(newfd, master); 
    
    if(verbose)
        printf("new sd := %d\n", newfd);

    if(newfd > *fdmax){ 
        *fdmax = newfd; 
    }
}

void close_connection(int sd, fd_set * master, int corrupted){
    if(!corrupted)
        close(sd);
    FD_CLR(sd, master);
}

void endpoint(int port, int(*__input)(char *, char **, int), char* (*__get_request)(char*, char **, int), int verbose_param){
    
    int res, listener, i, fdmax, params_len, j; 
    fd_set master, read_fds;

    char * answer, * command;
    char * params[MAX_PARAMS_LEN];

    char buf[BUF_LEN];

    verbose = verbose_param;

    listener = build_listener(port);
    
    if(listener <= 0){
        printf("\n\nerror port %d already in use, try with another one\n\n", port);
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

            memset(buf, 0, BUF_LEN);                                
            
            if(i == STDIN_FILENO){
                if(verbose)
                    printf("new input arrived\n");
                
                res = read(STDIN_FILENO, buf, BUF_LEN);
                
                if(res < 0){
                    if(verbose)
                        printf("error while typing\n");
                    exit(1);
                }

                buf[BUF_LEN - 1] = '\0';            

                /* get command */
                
                command = strtok(buf, " \t\n");
                params_len = 0;

                do{
                    params[params_len] = strtok(NULL, " \t\n");

                } while (params[params_len++] != NULL);

                params_len--;

                /* get command */

                /* close socket and exit */
                
                if(__input(command, params, params_len)){
                    close(listener);
                    exit(0);
                }

                continue;
            }

            res = receive_message(i, buf);

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
                printf("[%d] new message received := %s\n", i, buf);
            
            
            /* get command */   
                     
            command = strtok(buf, "|");
            params_len = 0;

            do{
                params[params_len] = strtok(NULL, "|");

            } while (params[params_len++] != NULL);

            params_len--;

            /* get command */
            
            answer = __get_request(command, params, params_len);

            if(answer == NULL)
                continue;
            
            res = send_message(i, answer); 
    
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

        }
    }

    close(listener);
}
