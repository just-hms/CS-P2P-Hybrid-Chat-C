#include "./lib/utils.h"
#include "./lib/endpoint.h"

#include "./lib/io.h"

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

    /* esc */

    if(strcmp(command, "esc") == 0){
        close_all_connections();
        return 1;
    }

    /* list all online users */

    if(strcmp(command, "list") == 0){
        
        user_list = user_get_online_list(1);
        
        if(user_list == NULL){
            printf("it's all quiet here...\n");
            return 0;
        }
        
        printf("%s\n", user_list);

        free(user_list);
        return 0;
    }

    /* help : prints help */

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
    connection_data * has_read_connection;

    int res, port;
    time_t t;
    char buf[BUF_LEN];

    char * response;

    if(request == NULL) 
        return NULL;

    /* signup|username|password */

    if(strcmp(request, "signup") == 0){
        
        if(len != 2) return build_string(ERROR_MESSAGE);

        if(strcmp(params[0], SERVER_NAME) == 0) 
            return build_string("not_available");

        if(strlen(params[0]) >= 50) 
            return build_string("too_long");
        
        res = user_add(params[0], params[1]);

        if(res) 
            return build_string("ok");

        return build_string("not_available");
    }

    /* in|username|password|port|old_logout_timestamp */

    if(strcmp(request, "in") == 0){
        
        if(len != 4) return build_string(ERROR_MESSAGE);

        if(!user_login(params[0], params[1])) 
            return build_string("wrong_user_or_password");
        
        /* get the old old_logout_timestamp */    

        sscanf(params[3], "%ld", &t);

        if(t != -1) user_end_session(params[0], t);

        port = user_get_session(params[0]);

        /* check if there's someone on the same port */

        if(port == -1){

            /* if yes close the connection with him */
            
            user_end_session(params[0], get_current_time());

            c = find_connection_by_username(params[0]);
            make_request(c, "connected_on_another_device", 0);
        }

        connection_set_username(sd, params[0]);
        user_start_session(params[0], atoi(params[2]));  
        return build_string("ok");

    }

    /* message|from|to|message|timestamp */

    if(strcmp(request, "message") == 0){
        
        if(len != 4) return build_string(ERROR_MESSAGE);
        
        /* if he doesn't exist error */

        if(!user_exists(params[1]))
            return build_string(ERROR_MESSAGE);
        
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

        /* send ACK with the port to sender */
        
        port = user_get_session(c->username);
        sprintf(buf, "%d", port);
        return build_string(buf);
    }

    /* hanging */

    if(strcmp(request, "hanging") == 0){
        
        if(len != 0) return build_string(ERROR_MESSAGE);
        
        c = find_connection_by_sd(sd);

        if(c == NULL || !c->logged) return build_string(ERROR_MESSAGE);
        
        return user_hanging(c->username);
    }

    /* show|username */

    if(strcmp(request, "show") == 0){
        
        if(len != 1) return build_string(ERROR_MESSAGE);
        
        if(!user_exists(params[0]))
            return ERROR_MESSAGE;
        
        c = find_connection_by_sd(sd);
        
        if(c == NULL || !c->logged)
            return build_string(ERROR_MESSAGE);
        
        response = user_show(c->username, params[0]);

        if(strlen(response) == 0)
            return build_string("");
        
        /* try to tell the sender that the receiver received the messages */
        
        has_read_connection = find_connection_by_username(params[0]);

        /* if he's offline buffer */

        if(has_read_connection == NULL){
            
            user_buffer_has_read(c->username, params[0]);
            return response;
        }
        
        /* otherwise send him the message has_read with the timestamp */

        sprintf(buf, "has_read|%s|%ld", c->username, get_current_time());

        make_request(
            has_read_connection,
            buf,
            0
        );

        return response;   
    }
    
    /* list */

    if(strcmp(request, "list") == 0){
        
        response = user_get_online_list(0);
        
        if(response == NULL)
            return build_string("it's all quiet here...");

        return response;
    }

    /* get_has_read|receiver */

    if(strcmp(request, "get_has_read") == 0){
        
        if(len != 1) return NULL;
        
        if(!user_exists(params[0]))
            return NULL;

        c = find_connection_by_sd(sd);
        
        if(c == NULL)
            return NULL;
        
        /* find out if the receiver has read the messages you sent him */

        t = user_get_buffered_has_read_time(c->username, params[0]);

        if(t != -1){
            sprintf(buf, "has_read|%s|%ld", params[0], t);
            make_request(c, buf, 0);
        }

        return NULL;
    }

    /* get_user_port|username */

    if(strcmp(request, "get_user_port") == 0){
        
        if(len != 1) return build_string(ERROR_MESSAGE);
        
        if(!user_exists(params[0]))
            return build_string(ERROR_MESSAGE);
        
        c = find_connection_by_username(params[0]);
        
        if(c == NULL) return build_string("-1");

        /* if he's online send the port in response */

        port = user_get_session(c->username);

        sprintf(buf, "%d", port);
        return build_string(buf);        
    }

    return NULL;
}

void disconnected_if_online(int sd){

    connection_data * c;
    char disconnected_username[50];

    c = find_connection_by_sd(sd);

    if(c == NULL) return;
    
    if(!c->logged) return;   
    
    strcpy(disconnected_username, c->username);

    /* set fake username to disconnected connection */

    connection_set_username(sd, SERVER_NAME);
 
    /* if someone else is logged with the same username don't end the session */

    if(find_connection_by_username(disconnected_username) != NULL) 
        return;

    /* if not just end the session */
    
    user_end_session(disconnected_username, get_current_time());
}

int main(int argc, char* argv[]){
     
    port = (argc == 2) ? atoi(argv[1]) : 4242;
    
    /* create fake server folder */

    user_create_folder(SERVER_NAME);
    system("clear");

    help();

    endpoint(
        port, 
        input, 
        get_request, 
        disconnected_if_online,
        1
    );

    return 1;
}