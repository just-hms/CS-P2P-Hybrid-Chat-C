#include "endpoint.h"

int verbose;
fd_set master;
int fdmax;
connection_data * head = NULL;
int count = 0;

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

connection_data * add_connection(int sd, int port){
    
    connection_data * new_connection;
    
    FD_SET(sd, &master); 
    if(sd > fdmax){ 
        fdmax = sd; 
    }

    new_connection = (connection_data *) malloc(sizeof(connection_data));
    new_connection->sd = sd;
    new_connection->port = port;
    new_connection->logged = 0;

    if(head == NULL){
        
        head = new_connection;
        head->next = NULL;
        count++;
        return new_connection;
    }

    new_connection->next = head;
    head = new_connection;
    count++;
    return new_connection;
}

connection_data * connection(int port){
    
    int res, sd;
    struct sockaddr_in srv_addr;
    connection_data * c;
    
    c = find_connection_by_port(port);
    
    if(c != NULL){
        return c;
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    res = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    
    if(res < 0)
        return NULL;

    return add_connection(sd, port);
}

void accept_new_connection(int listener){
    
    struct sockaddr_in cl_addr;
    int newfd, addrlen;

    addrlen = sizeof(cl_addr);
    
    newfd = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
    
    add_connection(newfd, -1);

    if(verbose)
        printf("new sd := %d\n", newfd);

}

void close_connection(int sd, int corrupted, void(*__disconnected) (int)){
    if(__disconnected != NULL)
        __disconnected(sd);

    connection_data * last;
    connection_data * cursor;
    
    if(!corrupted)
        close(sd);
    FD_CLR(sd, &master);

    if(head == NULL)
        return;
    
    cursor = head;
    last = NULL;

    if(head->sd == sd){

        head = head->next; 
        free(cursor);
        cursor = NULL;
        count--;
        return;
    }
    
    while (cursor){
        
        if(cursor->sd == sd){
            last->next = cursor->next;
            free(cursor);
            cursor = NULL;
            count--;
            return;
        }

        last = cursor;
        cursor = cursor->next;
    }
}

void endpoint(int port, int(*__input)(char *, char **, int, char *), char* (*__get_request)(char*, char **, int, int, char *), void(*__disconnected) (int), int verbose_param){
    
    int res, listener, i, params_len, j; 
    fd_set read_fds;


    char * answer, * command;
    char * params[MAX_PARAMS_LEN];
    char * raw_message;

    char buf[BUF_LEN];

    verbose = verbose_param;

    listener = build_listener(port);
    
    if(listener <= 0){
        
        system("clear");
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
                accept_new_connection(listener);
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
                
                raw_message = malloc(strlen(buf) * sizeof(char) + 1);
                strcpy(raw_message, buf);

                command = strtok(buf, " \t\n");
                params_len = 0;

                do{
                    params[params_len] = strtok(NULL, " \t\n");

                } while (params[params_len++] != NULL);

                params_len--;

                /* get command */
                
                if(__input(command, params, params_len, raw_message)){
                    close(listener);
                    free(raw_message);
                    exit(0);
                }   

                free(raw_message);
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
                close_connection(i, 1, __disconnected);
                continue;
            }

            if(verbose)
                printf("[%d] new message received := %s\n", i, buf);
            
            
            /* get command */   

            raw_message = malloc(strlen(buf) * sizeof(char) + 1);
            strcpy(raw_message, buf); 
            
            command = strtok(buf, "|");
            params_len = 0;

            do{
                params[params_len] = strtok(NULL, "|");

            } while (params[params_len++] != NULL);

            params_len--;

            /* get command */
                        
            answer = __get_request(command, params, params_len, i, raw_message);

            free(raw_message);
            
            if(answer == NULL)
                continue;

            res = send_message(i, answer); 

            if(res < 0){
                if(verbose)
                    printf("[%d] connection error while sending message\n", i);
                close_connection(i, 0, __disconnected);
                continue;
            }

            if(res != 0)
                continue;
            
            printf("[%d] closed connection while sending message\n", i);
            close_connection(i, 0, __disconnected);

        }
    }

    close(listener);
}

char * make_request(connection_data * connection, char * request, int need_response){
    
    int res;
    char * buf;

    if(connection == NULL)
        return NULL;

    res = send_message(connection->sd, request);

    if(res <= 0){
        close_connection(connection->sd, 1, NULL);
        return NULL;
    }

    if(need_response){
        
        buf = (char *) malloc(BUF_LEN);
        
        res = receive_message(connection->sd, buf);
        
        if(res <= 0){
            close_connection(connection->sd, 1, NULL);
            free(buf);
            return NULL;
        }

        return buf;
    }
    
    return NULL;
}

connection_data * find_connection_by_port(int port){

    connection_data * cursor;

    cursor = head;

    while (cursor){
        
        if(cursor->port == port)
            return cursor;

        cursor = cursor->next;
    }

    return NULL;
}

connection_data * find_connection_by_username(char * username){

    connection_data * cursor;

    cursor = head;

    while (cursor){
        
        if(strcmp(cursor->username,username) == 0)
            return cursor;

        cursor = cursor->next;
    }

    return NULL;
}

connection_data * find_connection_by_sd(int sd){

    connection_data * cursor;

    cursor = head;

    while (cursor){
        
        if(cursor->sd == sd)
            return cursor;

        cursor = cursor->next;
    }

    return NULL;
}

void close_all_connections(){
    connection_data * to_remove;

    while (head){
        to_remove = head;
        head = head->next;
        free(to_remove);
    }
    count = 0;
}

int count_connections(){
    return count;
}

void connection_set_username(int sd, char * username){
    connection_data * c;

    c = find_connection_by_sd(sd);

    if(c == NULL)
        return;
    
    strncpy(c->username, username, 50);
    
    c->logged = 1;
}