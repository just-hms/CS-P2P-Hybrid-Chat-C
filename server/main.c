#include "./../lib/endpoint.h"
#include "./../lib/utils.h"

/* TODO io.h ???*/

#include "./../lib/io.h"

void help(){
    printf("\thelp --> show commands details\n");
    printf("\tlist --> show user list \n");
    printf("\tesc --> shut down the server\n");

    printf("\noptionals:\n\n");
    printf("\tcls --> clear the screen\n\n");
}

int input(char * command, char ** params, int len){

    if(command == NULL){
        return 0;
    }
    if(strcmp(command, "esc") == 0){
        return 1;
    }

    if(strcmp(command, "list") == 0){
        /* list users */
        return 0;
    }

    if(strcmp(command, "help") == 0){
        help();
        return 0;
    }

    if(strcmp(command, "cls") == 0){
        system("clear");
        return 0;
    }

    printf("sorry %s is not a valid command\n", command);
    return 0;
}

char * get_request(char * request, char ** params, int len){
    
    int res, port;

    /* TODO maybe fix this*/
    char buf[BUF_LEN];
    
    if(request == NULL)
        return;

    if(strcmp(request, "signup") == 0){
        
        if(len != 2)
            return build_string("error");

        res = user_add(params[0], params[1]);

        if(res)
            return build_string("ok");

        return build_string("not_available");
    }

    if(strcmp(request, "in") == 0){
        
        if(len != 3)
            return build_string("error");

        if(user_login(params[0], params[1])){
            
            user_start_session(
                params[0],          /* username */
                atoi(params[2])     /* user_port */
            );

            return build_string("ok");
        }

        return build_string("wrong_user_or_password");
    }

    if(strcmp(request, "chat") == 0){
        
        if(len != 1)
            return build_string("error");
        
        /* if he doesn't exist error */
        if(!user_exists(params[0]))
            return build_string("error");
        
        port = user_get_session(params[0]);

        sprintf(buf, "%d", port);

        return build_string(buf);
    }
}

/* ./server [port] */


int main(int argc, char* argv[]){
    
    int port;

    port = (argc == 2) ? atoi(argv[1]) : 4040;
    
    system("clear");

    printf("server listening :%d\n\nType a command:\n\n", port);

    help();

    endpoint(
        port, 
        input, 
        get_request, 
        1
    );
}