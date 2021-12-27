#include "./../lib/endpoint.h"
#include "./../lib/io.h"

#include "./../lib/utils.h"

int default_port = 4242;                /* server default port*/

int current_port;                       /* your port */
char * current_username = NULL;         /* your current username */

int in_chat = 0;
char offline_username [50];             /* FIX ME */
char buf[BUF_LEN];

void add_to_chat(connection_data * c){
    /* TODO me*/
}

void open_chat(char * username){
    
    in_chat = 1;

    strcpy(offline_username, username);

    printf("\n\nin chat with {%s}\n\n", username);

    user_print_chat(current_username, username);
}

void handle_chat(char * command, char ** params, int len, char * raw){
    
    connection_data * c;
    
    /* TODO build message with command + params of find it somewhere */
    
    char * message;
    char * response;

    int port;

    /* quit the chat */    

    if(strcmp(command, "\\q") == 0){     
        in_chat = 1;
        return;
    }

    /* print list of online users */    

    if(strcmp(command, "\\u") == 0){
        
        if(len != 0){
            printf("error wrong format, type:\n\n\\u\n\n");
            return;
        }

        c = connection(default_port);

        response = make_request(
            c,
            "list",
            1
        );
        
        if(response == NULL){
            printf("error connectiong to the server\n");
            return;
        }

        printf("%s\n", response);

        return;
    }

    /* add user */    
    
    if(strcmp(command, "\\a") == 0){

        if(len != 1){
            printf("error wrong format, type:\n\n\\a username\n\n");
            return;
        }

        /* TODO create group chat with maybe an id */  
        
        if(strcmp(current_username, params[0]) ==  0){
            printf("error can't chat with yourself\n");
            return;
        }
        
        c = find_connection_by_username(params[0]);

        if(c != NULL){
            add_to_chat(c);
            return;
        }
        
        sprintf(buf, "get_user_port|%s\0", params[0]);

        c = connection(default_port);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return;
        }

        if(strcmp(response, "error") == 0){
            printf("sorry there is no one by the name of %s!\n", params[0]);
            return;
        }

        port = atoi(response);

        if(port == -1){
            printf("sorry %s is offline\n", params[0]);
            return;
        }

        c = connection(port);
        
        /* TODO */
        connection_set_username(c->sd, params[0]);
        add_to_chat(c);

        return;
    }

    /* write a message */
    
    /* TODO */

    c = find_connection_by_username(offline_username);

    sprintf(
        buf,
        "message|%s|%s|%s|%ld",
        current_username,
        offline_username,
        replace_n_with_0(raw),
        get_current_time()
    );

    if(c != NULL){
        make_request(
            c,
            buf,
            0
        );
        user_sent_message(current_username, offline_username, raw, get_current_time(), 1);
        return;
    }

    c = connection(default_port);

    response = make_request(
        c,
        buf,
        1
    );

    if(response == NULL){
        printf("sorry both the server and {%s} are offline...", offline_username);
        return;
    }
    if(strcmp(response, "error") == 0){
        printf("error comunicating with the server", offline_username);
        return;
    }
    if(strcmp(response, "offline") == 0){
        user_sent_message(current_username, offline_username, raw, get_current_time(), 0);
        return;
    }

    port = atoi(response);
    
    c = connection(port);

    if(c == NULL){
        user_sent_message(current_username, offline_username, raw, get_current_time(), 0);
        return;
    }

    connection_set_username(c->sd, offline_username);
    
    user_sent_message(current_username, offline_username, raw, get_current_time(), 1);

}

int input(char * command, char ** params, int len, char * raw){
    
    int res, port, i;
    char * response;
    connection_data * c;

    if(command == NULL)
        return 0;
    
    if(in_chat != 0){
        handle_chat(command, params, len, raw);
        return 0;
    }

    /* signup username password [port]*/

    if(strcmp(command, "signup") == 0){

        if(current_username != NULL){
            printf("you must logout to create an account\n");
            return 0;
        }      
        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\nsignup username password server_port\n\n");
            return 0;
        }

        sprintf(buf, "%s|%s|%s\0", command, params[0], params[1]);
                
        default_port = (len == 3) ? atoi(params[2]) : default_port;
        
        c = connection(default_port);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "ok") == 0){
            free(response);
            printf("congratulations %s, you're account has been created!\n", params[0]);

            return 0;
        }

        if(strcmp(response, "not_available") == 0){
            free(response);
            printf("sorry! %s is not available\n", params[0]);
            return 0;
        }

        free(response);
        printf("request error\n");

        return 0;
    }

    /* in username password [port] */

    if(strcmp(command, "in") == 0){
        
        if(current_username != NULL){
            printf("you must logout to login with another account\n");
            return 0;
        }      
        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\nin username password server_port\n\n");
            return 0;
        }
        
        sprintf(
            buf, 
            "%s|%s|%s|%d|%ld\0", 
            command, 
            params[0], 
            params[1], 
            current_port,
            get_out_time(params[0])
        );
    
        default_port = (len == 3) ? atoi(params[2]) : default_port;
        
        c = connection(default_port);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "ok") == 0){

            current_username = malloc(sizeof(params[0]) + sizeof(char));
            strcpy(current_username, params[0]);

            printf("congratulations {%s}, you're logged in!\n", params[0]);

            clear_out_time(params[0]);
            return 0;
        }
        
        if(strcmp(response, "wrong_user_or_password") == 0){
            printf("wrong username or password\n");
            return 0;
        }
        
        if(strcmp(response, "already_logged") == 0){
            printf("sorry but you are already logged in another device, disconnect from it to login from here\n");
            return 0;
        }

        free(response);
        printf("request error\n");

        return 0;
    }

    if(current_username == NULL){
        printf("you must do the login before doing any action beside login or signup\n");
        return 0;
    }

    /* hanging */

    if(strcmp(command, "hanging") == 0){
        
        if(len != 0){
            printf("error wrong format, type:\n\nhanging\n\n");
            return 0;
        }

        c = connection(default_port);
        
        sprintf(buf, "hanging|%s", current_username);
        
        response = make_request(
            c, 
            buf, 
            1
        );

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        printf("%s", response);

        return 0;
    }

    /* show username*/

    if(strcmp(command, "show") == 0){
        
        if(len != 1){
            printf("error wrong format, type:\n\nshow username\n\n");
            return 0;
        }

        c = connection(default_port);
        
        sprintf(buf, "hanging|%s|%s", params[0], current_username);
        
        response = make_request(
            c, 
            buf, 
            1
        );
        
        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        /* TODO
            add messages to chat
        */
        
        return 0;
    }

    /* chat username */
    if(strcmp(command, "chat") ==  0){

        if(len != 1){
            printf("error wrong format, type:\n\nchat username\n\n");
            return 0;
        }

        if(strcmp(current_username, params[0]) ==  0){
            printf("error can't chat with yourself\n");
            return 0;
        }
        
        if(!is_in_contacts(current_username, params[0])){
            printf("error %s is not in your contacts\n", params[0]);
            return 0;
        }

        open_chat(params[0]);

        return 0;
    }

    /* share file_path */

    if(strcmp(command, "share") == 0){
        
        /* TODO */
        
        return 0;
    }

    /* out */
    if(strcmp(command, "out") == 0){

        c = connection(default_port);

        if(c == NULL){
            save_out_time(current_username);
        }

        if(current_username != NULL)
            free(current_username);

        current_username = NULL;

        close_all_connections();
        return 1;
    }
    
    printf("sorry %s is not a valid command\n", command);
    
    return 0;
}

char * get_request(char * request, char ** params, int len, int sd, char * raw){
    
    time_t t;
    /* message|from|to|message|timestamp??? */
    
    if(strcmp(request, "message") == 0){

        /* TODO decide this*/

        if(len != 4){
            return NULL;
        }

        sscanf(params[3], "%ld", &t);

        user_received_message(current_username, params[0], params[2], t);

        if(in_chat == 0 || strcmp(params[0], offline_username) != 0){
            
            printf("you received a message from {%s}\n", params[0]);
            return NULL;
        }

        printf("%s := %s\n", params[0], params[2]);

        return NULL;
    }

    if(strcmp(request, "has_read") == 0){
        /* TODO 
            something in a function cause you could have to this in login or chat
        */
    }

}

/* ./client <port> */

void disconnected(int sd){
    connection_data * c;

    c = find_connection_by_sd(sd);

    if(c == NULL || c->port != default_port){
        return;
    }    
    printf("server disconnected...\n");

}

int main(int argc, char* argv[]){
    
    if(argc != 2){
        printf("error you must specify a port\n");
        exit(1);
    }
    
    current_port = atoi(argv[1]);

    endpoint(
        current_port, 
        input, 
        get_request, 
        disconnected,
        0
    );
}