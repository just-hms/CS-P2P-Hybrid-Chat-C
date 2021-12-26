#include "./../lib/utils.h"
#include "./../lib/connection.h"

#include "./../lib/io.h"

void help(){
    printf("\thelp --> show commands details\n");
    printf("\tlist --> show user list \n");
    printf("\tesc --> shut down the server\n");

    printf("optionals:\n");
    printf("\tcls --> clear the screen\n\n");
}

int input(char * command, char ** params, int len){
    
    char * user_list;

    if(command == NULL){
        return 0;
    }

    if(strcmp(command, "esc") == 0){
        return 1;
    }

    if(strcmp(command, "list") == 0){
        
        user_list = user_get_online_list(1);
        
        if(user_list == NULL){
            printf("it's all quiet here...\n", user_list);
            return 0;
        }
        printf("%s\n", user_list);
        free(user_list);
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

char * get_request(char * request, char ** params, int len, int sd){
    
    connection_data * c;
    int res, port;
    time_t t;

    /* FIX ME */

    char buf[BUF_LEN];

    char * response;
    
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
        
        if(len != 4)
            return build_string("error");

        if(user_login(params[0], params[1])){
            
            /* TEST */
            
            sscanf(params[3], "%ld", &t);

            if(t != -1){
                user_end_session(
                    params[0], 
                    t
                );
            }

            port = user_get_session(params[0]);
            
            if(port != -1)
                return build_string("already_logged");    

            user_start_session(
                params[0],          /* username */
                atoi(params[2]),    /* user_port */
                sd                  /* sd */
            );  

            return build_string("ok");
        }

        return build_string("wrong_user_or_password");
    }

    if(strcmp(request, "get_user_port") == 0){
        
        if(len != 1)
            return build_string("error");
        
        /* if he doesn't exist error */
        if(!user_exists(params[0]))
            return build_string("error");
        
        port = user_get_session(params[0]);

        sprintf(buf, "%d", port);

        return build_string(buf);
    }

    if(strcmp(request, "hanging") == 0){
        
        if(len != 1)
            return build_string("error");
            
        return user_hanging(params[0]);
    }

    if(strcmp(request, "show") == 0){
        
        if(len != 2)
            return build_string("error");
        
        if(!user_exists(params[0]))
            return build_string("error");

        response = user_show(params[0], params[1]);

        /* notify sender that the receiver has read his messages */
        
        port = user_get_session(params[0]);
        
        if(port == -1){
            
            /* TODO sender is offline */
            
            return response;
        }
        
        /* TODO flush shown messages */

        /* send notification to sender */
        
        return response;   
    }
    
    if(strcmp(request, "list") == 0){
        
        response = user_get_online_list(0);
        
        /* should never happen cause he's online */
        
        if(response == NULL){
            return build_string("it's all quiet here...");
        }

        return response;
    }

}

/* ./server [port] */

void disconnected_if_online(int sd){

    char * username;

    username = user_get_username_by_sd(sd);
    
    if(username == NULL)
        return;
    
    if(user_get_session(username) == -1){
        free(username);
        return;
    }

    user_end_session(
        username, 
        get_current_time()
    );

    free(username);    
}

int main(int argc, char* argv[]){
    
    int port;
    
    port = (argc == 2) ? atoi(argv[1]) : 4242;
    
    system("clear");

    printf("server listening :%d\n\nType a command:\n\n", port);

    help();

    endpoint(
        port, 
        input, 
        get_request, 
        disconnected_if_online,
        1
    );
}