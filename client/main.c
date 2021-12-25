#include "./../lib/endpoint.h"
#include "./../lib/connection.h"
#include "./../lib/io.h"

#include "./../lib/utils.h"

int logged_in = 0; /* TODO add this check for doing requests */

int default_port = 4040;

int current_port; /* your port */

connection_data * talking_to = NULL;
/* pass username, chatname, groupchat and others */

void open_chat(connection_data * tmp){
    
    char * chat;

    talking_to = tmp;

    chat = get_chat(tmp->username);

    printf("\n\nin chat with {%s}\n\n", tmp->username);

    printf("%s\n", chat);
    
    free(chat);
}

void handle_chat(char * command, char ** params, int len){
    
    connection_data * c;
    
    /* build message in some ways */
    char * message;

    /* quit the chat */    
    if(strcmp(command, "\\q") == 0){
        
        if(talking_to)
            free(talking_to);
        
        talking_to = NULL;
        return;
    }

    /* print list of online users */    
    if(strcmp(command, "\\u") == 0){

        return;
    }

    /* print add user */    
    if(strcmp(command, "\\a") == 0){

        return;
    }

    /* write a message */


    if(talking_to != NULL){
        
        request(
            talking_to,
            message,
            0
        );

        return;
    }
        
    c = connection(default_port, SERVER_NAME);
        
    if(c == NULL){
        
        printf("both the server and %s are offline\n", talking_to);

        if(talking_to)
            free(talking_to);
        talking_to = NULL;
        return;
    }
    
    /* buld message with "message" command + params */
    
    /* send message to server */

    request(
        c,
        params[0],
        0
    );
}

int input(char * command, char ** params, int len){
    
    int res, port, i;
    char * response;
    char buf[BUF_LEN];
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
        
        response = request(c, buf, 1);

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
        
        response = request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "ok") == 0){
            printf("congratulations {%s}, you're logged in!\n", params[0]);
            return 0;
        }
        if(strcmp(response, "wrong_user_or_password") == 0){
            printf("wrong username or password\n");
            return 0;
        }

        printf("request error\n");

        return 0;
    }

    if(strcmp(command, "hanging") == 0){
        
        return 0;
    }

    if(strcmp(command, "show") == 0){
        
        return 0;
    }

    if(strcmp(command, "chat") ==  0){

        if(len != 1){
            printf("error wrong format, type:\n\nchat username\n\n");
            return 0;
        }
        
        sprintf(buf, "%s|%s\0", command, params[0]);
        
        /* check if you are already connected with {username} */
        c = find_connection_by_username(params[0]);
        
        /* if yes open the chat with {username} */
        if(c != NULL){
            
            open_chat(c);
            return 0;
        }

        /* if no asks to the server */
        c = connection(default_port, SERVER_NAME);
        
        response = request(c, buf, 1);

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
            
            /* build fake connection */
            
            open_chat(NULL);
            return 0;
        }

        c = connection(port, params[0]);

        open_chat(c);

        return 0;
    }

    if(strcmp(command, "share") == 0){
        
        return 0;
    }

    if(strcmp(command, "out") == 0){
        
        /* TODO needs to always work even 
            if he doesn't write out
            server is offline 
        */

        close_all_connections();
        return 1;
    }
    
    printf("error wrong format\n");
    
    return 0;
}

char * get_request(char * request, char ** params, int len){
    
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