#include "./../lib/server.c"
#include "./../lib/client.c"

#include "../lib/utils.h"

int in_chat = 0;

void handle_chat(char * input){
    
    if(starts_with(input, "\\q")){
        /* TODO something else ??? */
        in_chat = 0;
        return;
    }

    if(starts_with(input, "\\u")){

        return;
    }

    if(starts_with(input, "\\a")){

        return;
    }

}

void input(char * input){
    
    int ret, port;
    
    char cmd[1024], username[1024], password[1024];
    char * response;

    if(input == NULL)
        return;
    
    if(in_chat){
        handle_chat(input);
        return;
    }

    /* signup username password [port]s*/
    
    if(starts_with(input, "signup")){

        sscanf(input, "%1024s %1024s %1024s %d", cmd, username, password, port);
        
        if(strlen(username) == 0 || strlen(password) == 0){
            
            printf("ERROR: wrong format\n");
            
            return;
        }

        if(port == NULL){
            
        }
        
        return;

        if(ret < 0){
            printf("ERROR: connectiong to the server\n");
            return;
        }

        if(starts_with(response, "ok")){
            printf("congratulations %d, you're account has been created!\n", username);
            return;
        }
        if(starts_with(response, "error")){
            printf("sorry! %d is not available\n", username);
            return;
        }

        return;
    }

    /* in username password [port] */

    if(starts_with(input, "in")){
        
        return;
    }
    if(starts_with(input, "hanging")){
        
        return;
    }
    if(starts_with(input, "show")){
        
        return;
    }

    if(starts_with(input, "chat")){
        
        return;
    }

    if(starts_with(input, "share")){
        
        return;
    }

    if(starts_with(input, "out")){
        
        return;
    }

}

char * get_request(char * request){
    
}

int main(int argc, char* argv[]){
    
    /* TODO get input for the port listening to */
    
    if(argc == 0){
        printf("ERROR: you must specifie a port to connect to!");
        exit(1);
    }

    /*input check */

    server(
        atoi(argv[0]), 
        input, 
        get_request, 
        0
    );
}