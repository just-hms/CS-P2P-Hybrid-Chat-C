#include "./../lib/endpoint.h"
#include "./../lib/connection.h"

#include "../lib/utils.h"

int in_chat = 0;

void handle_chat(char * command, char ** params, int len){
    
    if(strcmp(command, "\\q") == 0){
        /* TODO something else ??? */
        in_chat = 0;
        return;
    }

    if(strcmp(command, "\\u") == 0){

        return;
    }

    if(strcmp(command, "\\a") == 0){

        return;
    }

    printf("error wrong format\n");
}

void input(char * command, char ** params, int len){
    
    int res, port, i;
    char * response;
    char buffer[1024];
    connection_data * c;

    if(command == NULL)
        return;
    
    if(in_chat){
        handle_chat(command, params, len);
        return;
    }

    /* signup username password [port]*/
    
    if(strcmp(command, "signup") == 0){
                
        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\n\tsignup username password\n\n");
            return;
        }

        sprintf(buffer, "%s %s %s", command, params[0], params[1]);
        
        response = request(
            connection(port, SERVER_NAME),
            buffer,
            &res,
            1
        );

        if(res <= 0){
            printf("error connectiong to the server\n");
            return;
        }

        if(strcmp(response, "ok") == 0){
            printf("congratulations %d, you're account has been created!\n", params[0]);
            return;
        }

        if(strcmp(response, "not_available") == 0){
            printf("sorry! %d is not available\n", params[0]);
            return;
        }
        
        printf("request error\n");
        
        return;
    }

    /* in username password [port] */

    if(strcmp(command, "in") == 0){
        
        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\n\tin username password\n\n");
            return;
        }
        
        port = (len == 3) ? atoi(params[2]) : 4040;
        
        sprintf(buffer, "%s %s %s", command, params[0], params[1]);

        response = request(
            connection(port, SERVER_NAME),
            buffer,
            &res,
            1
        );

        if(res <= 0){
            printf("error connecting to the server\n");
            return;
        }

        if(strcmp(response, "ok") == 0){
            printf("congratulations {%d}, you're logged in!\n", params[0]);
            return;
        }
        if(strcmp(response, "wrong_user_or_password") == 0){
            printf("wrong username or password\n", params[0]);
            return;
        }

        printf("request error\n");

        return;
    }

    if(strcmp(command, "hanging") == 0){
        
        return;
    }

    if(strcmp(command, "show") == 0){
        
        return;
    }

    if(strcmp(command, "chat") ==  0){
        
        return;
    }

    if(strcmp(command, "share") == 0){
        
        return;
    }

    if(strcmp(command, "out") == 0){
        
        return;
    }
    
    printf("error wrong format\n");

}

char * get_request(char * request, char ** params, int len){
    
}

/* ./client <port> */

int main(int argc, char* argv[]){
    
    if(argc != 2){
        printf("error you must specify a port\n");
        exit(1);
    }
    
    endpoint(
        atoi(argv[1]), 
        input, 
        get_request, 
        0
    );
}