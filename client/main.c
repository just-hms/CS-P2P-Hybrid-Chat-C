#include "./../lib/endpoint.h"
#include "./../lib/io.h"

#include "./../lib/utils.h"

int default_port = 4242;                /* server default port*/

int current_port;                       /* your port */
char * current_username = NULL;         /* your current username */

int in_chat = 0;
char talking_to [50];                   /* FIX ME */
char buf[BUF_LEN];

void add_to_chat(connection_data * c){
    /* TODO me*/
}

void refresh_chat(){
    if(!in_chat)
        return;
    
    system("clear");

    printf("\n\nin chat with {%s}\n\n", talking_to);

    user_print_chat(current_username, talking_to);
}
void open_chat(char * username){
    
    in_chat = 1;
    strcpy(talking_to, username);
    refresh_chat();
}

void handle_chat(char * command, char ** params, int len, char * raw){
    
    connection_data * c;
    
    char * message;
    char * response;

    int port;

    /* quit the chat */    

    if(strcmp(command, "\\q") == 0){     
        in_chat = 0;
        system("clear");
        /* TODO show commands ???*/
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
        
        connection_set_username(c->sd, params[0]);
        add_to_chat(c);

        return;
    }

    /* write a message */
    
    c = find_connection_by_username(talking_to);

    sprintf(
        buf,
        "message|%s|%s|%s|%ld",
        current_username,
        talking_to,
        replace_n_with_0(raw),
        get_current_time()
    );

    if(c != NULL){
        make_request(
            c,
            buf,
            0
        );
        user_sent_message(current_username, talking_to, raw, get_current_time(), 1);
        refresh_chat();
        return;
    }

    c = connection(default_port);

    response = make_request(
        c,
        buf,
        1
    );

    if(response == NULL){
        printf("sorry both the server and {%s} are offline...", talking_to);
        return;
    }
    if(strcmp(response, "error") == 0){
        printf("error comunicating with the server", talking_to);
        return;
    }
    if(strcmp(response, "offline") == 0){
        user_sent_message(current_username, talking_to, raw, get_current_time(), 0);
        refresh_chat();
        return;
    }

    port = atoi(response);
    
    c = connection(port);

    if(c == NULL){
        user_sent_message(current_username, talking_to, raw, get_current_time(), 0);
        refresh_chat();
        return;
    }

    connection_set_username(c->sd, talking_to);
    
    sprintf(buf, "im|%s\0", current_username);

    make_request(
        c,
        buf,
        0
    );

    user_sent_message(current_username, talking_to, raw, get_current_time(), 1);
    refresh_chat();
}

int input(char * command, char ** params, int len, char * raw){
    
    int res, port, i;
    char * response;
    char * message;
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
            printf("congratulations {%s}, you're account has been created!\n", params[0]);

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
            
            connection_set_username(c->sd, SERVER_NAME);
            
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
        
        response = make_request(
            c, 
            "hanging", 
            1
        );

        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        /* FIX ME */

        if(strnlen(response, 2) == 0){
            printf("no message pending...\n");
            return 0;
        }
        
        printf("%s\n", response);

        return 0;
    }

    /* show username*/

    if(strcmp(command, "show") == 0){
        
        if(len != 1){
            printf("error wrong format, type:\n\nshow username\n\n");
            return 0;
        }

        if(strcmp(params[0], current_username) == 0){
            printf("can't show messages from you\n");
            return 0;
        }
        c = connection(default_port);
        
        sprintf(buf, "show|%s", params[0]);
        
        response = make_request(
            c, 
            buf, 
            1
        );
        
        if(response == NULL){
            printf("error connectiong to the server\n");
            return 0;
        }

        if(strcmp(response, "error") == 0){
            printf("sorry the username you specified doesn't exist\n");
            return 0;
        }

        message = strtok(response, "\n");

        while (message){
            
            printf("%s := %s\n", params[0], message);

            /* TODO server should send the timestamp */

            user_received_message(current_username, params[0], message, get_current_time());
            
            message = strtok(NULL, "\n");
        }
        
        
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

        /* if server is online check if there are some buffered read messaged notification */

        c = connection(default_port);
        
        sprintf(buf, "get_has_read|%s", params[0]);

        make_request(
            c,
            buf,
            0
        );

        /* open the chat anyway */

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
            printf("saving out time...\n");
            save_out_time(current_username);
        }

        close_all_connections();
        return 1;
    }
    
    printf("sorry %s is not a valid command\n", command);
    
    return 0;
}

char * get_request(char * request, char ** params, int len, int sd, char * raw){
    
    time_t t;

    /* message|from|to|message|timestamp??? */
    
    if(request == NULL){
        return NULL;
    }

    if(strcmp(request, "message") == 0){

        if(len != 4){
            return NULL;
        }

        sscanf(params[3], "%ld", &t);

        user_received_message(current_username, params[0], params[2], t);

        if(in_chat == 0 || strcmp(params[0], talking_to) != 0){
            
            printf("you received a message from {%s}\n", params[0]);
            return NULL;
        }

        refresh_chat();
        return NULL;
    }

    if(strcmp(request, "has_read") == 0){

        if(len != 2)
            return NULL;
        
        sscanf(params[1], "%ld", &t);

        user_has_read(current_username, params[0], t);

        if(in_chat && strcmp(talking_to, params[0]) == 0)
            refresh_chat();

        return NULL;
    }

    if(strcmp(request, "im") == 0){

        if(len != 1)
            return NULL;

        connection_set_username(sd, params[0]);

        return NULL;
    }

}

/* ./client <port> */

void disconnected(int sd){
    
    connection_data * c;

    c = find_connection_by_sd(sd);

    printf("[%d] disconnected\n", c->sd);
    
    if(c == NULL || c->port != default_port){
        return;
    }    

    /* printf("server disconnected...\n"); */

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