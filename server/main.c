#include "./../lib/utils.h"
#include "./../lib/endpoint.h"

#include "./../lib/io.h"

int port;

void help(){
    printf("server listening... :%d\n\n", port);
    printf("Type a command:\n");
    printf("\thelp --> show commands details\n");
    printf("\tlist --> show user list \n");
    printf("\tesc --> shut down the server\n");

    printf("optionals:\n");
    printf("\tcls --> clear the screen\n");
}

int input(char * command, char ** params, int len, char * raw){
    
    char * user_list;

    if(command == NULL){
        return 0;
    }

    if(strcmp(command, "esc") == 0){
        close_all_connections();
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

char * get_request(char * request, char ** params, int len, int sd, char * raw){
    
    connection_data * c;
    connection_data * c_1;
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

            connection_set_username(sd, params[0]);

            user_start_session(
                params[0],          /* username */
                atoi(params[2])     /* user_port */
            );  

            return build_string("ok");
        }

        return build_string("wrong_user_or_password");
    }

    /* message|from|to|message|timestamp??? */

    if(strcmp(request, "message") == 0){
        
        if(len != 4)
            return build_string("error");
        
        /* if he doesn't exist error */

        if(!user_exists(params[1]))
            return build_string("error");
        
        c = find_connection_by_username(params[1]);
        
        if(c == NULL){
            
            /* buffer the message */

            printf("buffering := %s\n", raw);

            sscanf(params[3], "%ld", &t);
            user_buffer_message(params[0], params[1], params[2], t);
            
            return build_string("offline");
        }

        /* forward the message */

        printf("forwarding := %s\n", raw);

        make_request(
            c,
            raw,
            0
        );

        /* send ACK */
        
        port = user_get_session(c->username);
        sprintf(buf, "%d", port);
        return build_string(buf);
    }

    if(strcmp(request, "hanging") == 0){
        
        if(len != 0)
            return build_string("error");
        
        c = find_connection_by_sd(sd);

        if(c == NULL || !c->logged)
            return build_string("error");
        
        return user_hanging(c->username);
    }

    if(strcmp(request, "show") == 0){
        
        if(len != 1)
            return build_string("error");
        
        if(!user_exists(params[0]))
            return "error";
        
        c = find_connection_by_sd(sd);
        
        if(c == NULL || !c->logged)
            return build_string("error");
        
        response = user_show(c->username, params[0]);

        if(response == "")
            return "no_message_found";

        c_1 = find_connection_by_username(params[0]);

        if(c_1 == NULL){
            
            user_buffer_has_read(c->username, params[0]);
            
            return response;
        }
        
        sprintf(buf, "has_read|%s|%ld", c->username, get_current_time());

        make_request(
            c_1,
            buf,
            0
        );

        return response;   
    }
    
    if(strcmp(request, "list") == 0){
        
        response = user_get_online_list(0);
        
        /* should never happen cause he's online */
        
        if(response == NULL)
            return build_string("it's all quiet here...");

        return response;
    }

    if(strcmp(request, "get_has_read") == 0){
        
        if(len != 1)
            return NULL;
        
        c = find_connection_by_sd(sd);

        if(!user_exists(params[0]))
            return NULL;
        
        if(c == NULL)
            return NULL;
        
        t = user_get_buffered_has_read_time(c->username, params[0]);

        if(t != -1){
            printf("lol\n");

            sprintf(buf, "has_read|%s|%ld", params[0], t);
            
            make_request(
                c,
                buf,
                0
            );
        }

        return NULL;
    }

    if(strcmp(request, "get_user_port") == 0){
        
        if(len != 1)
            return build_string("error");
        
        if(!user_exists(params[0]))
            return build_string("error");
        
        c = find_connection_by_username(params[0]);
        
        if(c != NULL){
            port = user_get_session(c->username);
            sprintf(buf, "%d", port);
            return build_string(buf);
        }
        
        return build_string("-1");
    }
}

/* ./server [port] */

void disconnected_if_online(int sd){

    connection_data * c;

    c = find_connection_by_sd(sd);
    
    if(c == NULL)
        return;
    
    if(!c->logged){
        return;
    }

    user_end_session(
        c->username, 
        get_current_time()
    );

}

int main(int argc, char* argv[]){
     
    port = (argc == 2) ? atoi(argv[1]) : 4242;
    
    system("clear");

    help();

    endpoint(
        port, 
        input, 
        get_request, 
        disconnected_if_online,
        1
    );
}