#include "./../lib/endpoint.h"
#include "./../lib/connection.h"
#include "./../lib/io.h"

#include "./../lib/utils.h"

int default_port = 4040;

int current_port;                       /* your port */
char * current_username = NULL;         /* your current username */

connection_data * talking_to = NULL;
char offline_username [50];             /* FIX ME */
char buf[BUF_LEN];

void add_to_chat(connection_data * c){
    /* TODO me*/
}

void open_chat(char * username){
    
    char * chat;

    /* FIX ME */
    
    strcpy(offline_username, username);

    chat = get_chat(username);

    if(talking_to)
        printf("\n\nin chat with {%s} :online\n\n", username);
    else
        printf("\n\nin chat with {%s} :offline\n\n", username);
    
    printf("%s\n", chat);
    
    free(chat);
}

void handle_chat(char * command, char ** params, int len){
    
    connection_data * c;
    
    /* TODO build message with command + params of find it somewhere */
    
    char * message;
    char * response;

    int port;

    /* quit the chat */    

    if(strcmp(command, "\\q") == 0){     
        talking_to = NULL;
        return;
    }

    /* print list of online users */    

    if(strcmp(command, "\\u") == 0){
        
        if(len != 0){
            printf("error wrong format, type:\n\n\\u\n\n");
            return;
        }

        c = connection(default_port, SERVER_NAME);

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

        /* TOOD create group chat with maybe an id */  
        
        c = find_connection_by_username(params[0]);

        if(c != NULL){
            add_to_chat(c);
            return;
        }
        
        sprintf(buf, "get_user_port|%s\0", params[0]);

        c = connection(default_port, SERVER_NAME);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "error") == 0){
            printf("sorry there is no one by the name of %s!\n", params[0]);
            return 0;
        }

        port = atoi(response);

        if(port == -1){
            printf("sorry %s is offline\n", params[0]);
            return 0;
        }

        c = connection(params[0], port);
        add_to_chat(c);

        return 0;
    }

    /* write a message */

    if(talking_to != NULL){
        
        /* message|from|to|message|timestamp??? */

        make_request(
            talking_to,
            message,
            0
        );

        return;
    }
        
    c = connection(default_port, SERVER_NAME);
        
    if(c == NULL){
        
        printf("both the server and %s are offline\n", talking_to);
        talking_to = NULL;
        return;
    }
    
    /* buld message with message|from|to|message|timestamp??? */
    
    make_request(
        c,
        message,
        0
    );
}

int input(char * command, char ** params, int len){
    
    int res, port, i;
    char * response;
    connection_data * c;

    if(command == NULL)
        return 0;
    
    if(talking_to != NULL){
        handle_chat(command, params, len);
        return 0;
    }

    /* signup username password [port]*/
    
    if(strcmp(command, "signup") == 0){
                
        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\nsignup username password server_port\n\n");
            return 0;
        }

        sprintf(buf, "%s|%s|%s\0", command, params[0], params[1]);
                
        default_port = (len == 3) ? atoi(params[2]) : default_port;
        
        c = connection(default_port, SERVER_NAME);
        
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
        
        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\nin username password server_port\n\n");
            return 0;
        }

        sprintf(buf, "%s|%s|%s|%d\0", command, params[0], params[1], current_port);
        
        default_port = (len == 3) ? atoi(params[2]) : default_port;
        
        c = connection(default_port, SERVER_NAME);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "ok") == 0){

            current_username = malloc(sizeof(params[0]) + sizeof(char));
            strcpy(current_username, params[0]);

            printf("congratulations {%s}, you're logged in!\n", params[0]);
            return 0;
        }
        
        if(strcmp(response, "wrong_user_or_password") == 0){
            printf("wrong username or password\n");
            return 0;
        }
        
        if(strcmp(response, "already_logged") == 0){
            printf("sorry but you are already logged\n");
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

        c = connection(default_port, SERVER_NAME);
        
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

        c = connection(default_port, SERVER_NAME);
        
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
        
        
        /* check if you are already connected with {username} */

        c = find_connection_by_username(params[0]);
        
        /* if yes open the chat with {username} */

        if(c != NULL){
            talking_to = c;
            open_chat(params[0]);
            return 0;
        }

        sprintf(buf, "get_user_port|%s\0", params[0]);

        /* if no asks to the server */
        
        c = connection(default_port, SERVER_NAME);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "error") == 0){
            printf("sorry there is no one by the name of %s!\n", params[0]);
            return 0;
        }

        port = atoi(response);

        /* peer offline */
        if(port == -1){
            open_chat(params[0]);
            return 0;
        }

        c = connection(port, params[0]);

        talking_to = c;
        open_chat(c->username);
        return 0;
    }

    /* share file_path */

    if(strcmp(command, "share") == 0){
        
        /* TODO */
        
        return 0;
    }

    /* out */
    if(strcmp(command, "out") == 0){
        
        /* TODO 
            needs to always work even 
            if he doesn't write out
            server is offline 
        */

        if(current_username)
            free(current_username);
        current_username = NULL;
        
        close_all_connections();
        return 1;
    }
    
    printf("sorry %s is not a valid command\n", command);
    
    return 0;
}

char * get_request(char * request, char ** params, int len){
    
    /* message|from|to|message|timestamp??? */
    
    if(strcmp(request, "message") == 0){

        /* TODO decide this*/

        if(len != 3){
            return NULL;
        }
        if(talking_to == NULL){
            /* notify and refresh chat */
            return NULL;
        }

        if(strcmp(params[0], talking_to->username) != 0 ){
            /* notify and refresh chat */
            return NULL;
        }
        
        printf("%s := %s\n", params[1], params[2]);

        return NULL;
    }

    if(strcmp(request, "has_read") == 0){
        /* TODO 
            something in a function cause you could have to this in login or chat
        */
    }

}

/* ./client <port> */

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
        0
    );
}