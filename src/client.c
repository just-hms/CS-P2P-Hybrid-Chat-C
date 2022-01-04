#include "./lib/endpoint.h"
#include "./lib/io.h"

#include "./lib/utils.h"

struct chat_data{
    char username[USERNAME_LENGTH];
    int online;
    struct chat_data * next;    
} typedef chat_data;

int server_port = 4242;                 /* server port*/

int current_port;                       /* your port */
char * current_username = NULL;         /* your current username */

int talking_to_count = 0;
chat_data * talking_to = NULL;
time_t group_id = -1;
char buf[BUF_LEN];

int in_group() { return group_id != -1; }
int in_chat() { return in_group() || talking_to_count > 0; }
int logged() { return current_username != NULL; }

int already_talking_to(char * username){

    chat_data * cursor = talking_to;

    while (cursor){
        if(strcmp(cursor->username, username) == 0) return 1;
        cursor = cursor->next;
    }

    return 0;
}

void chat_help(){
    printf("\n\\q --> close the chat\n");
    printf("\\a --> to add a friend\n");
    printf("\\u --> to show a list of online users\n");
    printf("share filename --> to share a filename with you're friends\n\n");
}

void commands_help(){

    if(current_username == NULL){
        printf("\nsignup username password [port] --> to create an account\n");
        printf("in username password [port] --> to login\n");
        printf("out --> close the application\n\n");
        return;
    }
    printf("\nhey {%s}\n", current_username);
    printf("\nchat username --> to chat with someone from your contacts\n");
    printf("hanging --> to see all your pending messages\n");
    printf("show username --> to see the pending messages from username\n");
    printf("show username --> to see the pending messages from username\n");
    printf("out --> logout and close the application\n\n");

}

void help(){

    if(!in_chat()){
        commands_help();
        return;
    }

    chat_help();
}

void clear_chatters(){
    
    chat_data * cursor;
    chat_data * to_delete;
    
    connection_data * c;

    if(in_group()){
        
        for (cursor = talking_to; cursor; cursor = cursor->next){

            c = find_connection_by_username(cursor->username);
            make_request(c, "group_quit", 0);    
        }
    }
    

    cursor = talking_to;

    while (cursor){
        to_delete = cursor;
        cursor = cursor->next;
        free(to_delete);
    }

    talking_to_count = 0;
    group_id = -1;
    talking_to = NULL;
    
}

void refresh_chat(){
    
    chat_data * cursor;

    system("clear");

    if(!in_group() && talking_to_count == 1){
        
        printf("{%s} | in chat with {%s}\n\n", current_username, talking_to->username);
        user_print_chat(talking_to->username);
        help();
        return;
    }
    
    /* group chat */

    if(talking_to == 0){

        printf("{%s} | in group chat alone\n\n", current_username);
        user_print_group_chat(group_id);
        help();
        return;
    }

    printf("{%s} | in group chat with {", current_username);

    for (cursor = talking_to; cursor; cursor = cursor->next)
        printf("%s, ", cursor->username);
    
    printf("\b\b}\n\n");
    user_print_group_chat(group_id);
    help();
}

void group_quit(connection_data * c){
    chat_data * cursor;
    chat_data * last;

    if(talking_to == NULL){
        refresh_chat();
        return;
    }

    cursor = talking_to;

    /* head delete */

    if(strcmp(cursor->username, c->username) == 0){
        talking_to  = talking_to->next;
        talking_to_count--;
        free(cursor);
        cursor = NULL;
        refresh_chat();
        return;
    }

    while (cursor){
        
        if(strcmp(cursor->username, c->username) == 0){

            last->next = cursor->next;
            talking_to_count--;
            free(cursor);
            cursor = NULL;
            refresh_chat();
            return;
        }

        last = cursor;
        cursor = cursor->next;
    }

    refresh_chat();
    
}

/*
    add a chatter
    echo == 1 --> don't just add him to the chatters but send him messages
    echo == 0 --> you've been added and you passively add him to chat 
*/

void add_to_chat(char * username, connection_data * new_member, int echo){

    chat_data * cursor;
    chat_data * new_chatter;

    if(strcmp(current_username, username) ==  0){
        printf("error can't chat with yourself\n");
        return;
    }

    new_chatter = malloc(sizeof(chat_data));
    strcpy(new_chatter->username, username);

    if(talking_to_count == 0){
        talking_to = new_chatter;
        talking_to->next = NULL;
        talking_to_count++;

        /* if you add someone for the first time but you are in a group tell him that he's been added to one */

        if(in_group() && echo){
            sprintf(buf, "group_member|%ld|%s|%d", group_id, current_username, current_port);
            
            make_request(
                find_connection_by_username(talking_to->username), 
                buf, 
                0
            );
        }
        refresh_chat();
        return;
    }

    if(!in_group()){

        if(find_connection_by_username(talking_to->username) == NULL){
            printf("sorry to start a group chat make sure that who you're talking to is online...\n");
            return;
        }

        group_id = get_current_time();
    }

    talking_to_count++;
    
    if(!echo){
        new_chatter->next = talking_to;
        talking_to = new_chatter;
        refresh_chat();
        return;
    }

    /* tell everyone in the group that you added new_member */

    for (cursor = talking_to; cursor; cursor = cursor->next){

        sprintf(buf, "group_add|%ld|%s|%d", group_id, new_member->username, new_member->port);
        
        make_request(
            find_connection_by_username(cursor->username), 
            buf, 
            0
        );
    }    

    /* tell the new member who you are and your data */

    sprintf(buf, "group_member|%ld|%s|%d", group_id, current_username, current_port);

    make_request(new_member, buf, 0);
    
    new_chatter->next = talking_to;
    talking_to = new_chatter;
    refresh_chat();
}

void handle_chat(char * command, char ** params, int len, char * raw){
    
    connection_data * c;
    chat_data * cursor;
    
    char * response;

    int port;

    /* quit the chat */    

    if(strcmp(command, "\\q") == 0){     
        if(len != 0){
            printf("error wrong format, type:\n\n\\q\n\n");
            return;
        }
        system("clear");
        clear_chatters();
        help();
        return;
    }

    /* share filename */

    if(strcmp(command, "share") == 0){
        if(len != 1){
            printf("error wrong format, type:\n\nshare filename\n\n");
            return;
        }

        if(!in_group() && find_connection_by_username(talking_to->username) == NULL){
            printf("sorry to share a file make sure that who you're talking to is online...\n");
            return;
        }
        
        for (cursor = talking_to; cursor; cursor = cursor->next)
            send_file(
                find_connection_by_username(cursor->username), 
                params[0]
            );

        printf("sent!\n");
        return;
        
    }

    /* print list of online users */    

    if(strcmp(command, "\\u") == 0){
        
        if(len != 0){
            printf("error wrong format, type:\n\n\\u\n\n");
            return;
        }

        c = connection(server_port);

        response = make_request(c,"list", 1);
        
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
        
        c = find_connection_by_username(params[0]);
        
        if(already_talking_to(params[0])){
            printf("error you're already talking to {%s}\n", params[0]);
            return;
        }

        /* if you're already connected with him just add him to the chat */
        
        if(c != NULL){
            add_to_chat(c->username, c, 1);
            return;
        }

        /* otherwise ask the server for his port */

        sprintf(buf, "get_user_port|%s", params[0]);

        c = connection(server_port);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            return;
        }

        if(strcmp(response, ERROR_MESSAGE) == 0){
            printf("sorry there is no one by the name of {%s}!\n", params[0]);
            return;
        }

        port = atoi(response);

        if(port == -1){
            printf("sorry {%s} is offline\n", params[0]);
            return;
        }

        c = connection(port);
        
        connection_set_username(c->sd, params[0]);
        add_to_chat(c->username, c, 1);

        return;
    }

    /* write a message */ 

    if(!in_group()){

        c = find_connection_by_username(talking_to->username);

        sprintf(
            buf,
            "message|%s|%s|%s|%ld",
            current_username,
            talking_to->username,
            raw,
            get_current_time()
        );

        /*  
            if you already send a memssage to the receiver 
            and he is online send the message to him 
        */

        if(c != NULL){
            make_request(c, buf, 0);
            user_sent_message(talking_to->username, raw, get_current_time(), 1);
            refresh_chat();
            return;
        }

        /*
            if not send the message to the server
            and wait for the ack
        */

        c = connection(server_port);

        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("sorry both the server and {%s} are offline...", talking_to->username);
            return;
        }

        if(strcmp(response, ERROR_MESSAGE) == 0){
            printf("error comunicating with the server");
            return;
        }

        if(strcmp(response, "offline") == 0){
            user_sent_message(talking_to->username, raw, get_current_time(), 0);
            refresh_chat();
            return;
        }
        /* if he's online the server sent him the message and sent you the ACK with the port*/

        port = atoi(response);
        
        /* 
            now that you know that he's online connect to him 
            and tell him who you are
        */

        c = connection(port);

        /* this shouldn't happen */
        if(c == NULL){
            user_sent_message(talking_to->username, raw, get_current_time(), 0);
            refresh_chat();
            return;
        }

        connection_set_username(c->sd, talking_to->username);
        
        sprintf(buf, "im|%s|%d", current_username, current_port);

        make_request(c, buf, 0 );

        user_sent_message(talking_to->username, raw, get_current_time(), 1);
        refresh_chat();
        
        return;
    }

    /* send group message */

    sprintf(
        buf,
        "group_message|%s|%s",
        current_username,
        raw
    );    
    
    for (cursor = talking_to; cursor; cursor = cursor->next)
        make_request(
            find_connection_by_username(cursor->username),
            buf,
            0
        );
    
    user_sent_group_message(group_id, raw);
    refresh_chat();
}

int input(char * command, char ** params, int len, char * raw){
    
    char * response;
    char * message;
    connection_data * c;

    if(command == NULL)
        return 0;

    if(in_chat()){
        handle_chat(command, params, len, raw);
        return 0;
    }

    system("clear");
    
    /* signup username password [port] */

    if(strcmp(command, "signup") == 0){

        if(logged()){
            printf("you must logout to create an account\n");
            help();
            return 0;
        }

        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\nsignup username password server_port\n\n");
            help();
            return 0;
        }

        sprintf(buf, "%s|%s|%s", command, params[0], params[1]);

        /* if specified get the port */

        server_port = (len == 3) ? atoi(params[2]) : server_port;
        
        c = connection(server_port);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            help();
            return 0;
        }

        if(strcmp(response, "ok") == 0){
            free(response);
            printf("congratulations {%s}, you're account has been created!\n", params[0]);
            help();
            return 0;
        }

        if(strcmp(response, "not_available") == 0){
            free(response);
            printf("sorry! %s is not available\n", params[0]);
            help();
            return 0;
        }

        if(strcmp(response, "too_long") == 0){
            free(response);
            printf("sorry! the username must be shorter than 50 characters\n");
            help();
            return 0;
        }

        free(response);
        printf("request error\n");
        help();

        return 0;
    }

    /* in username password [port] */

    if(strcmp(command, "in") == 0){
        
        if(logged()){
            printf("you must logout to login with another account\n");
            help();
            return 0;
        }      

        if(len < 2 || len > 3){
            printf("error wrong format, type:\n\nin username password server_port\n\n");
            help();
            return 0;
        }
        
        sprintf(
            buf, 
            "in|%s|%s|%d|%ld", 
            params[0],                  /* username */ 
            params[1],                  /* password */
            current_port,               /* port */
            get_out_time(params[0])     /* old out time (-1 if not exists)*/
        );

        /* get the port if specified */

        server_port = (len == 3) ? atoi(params[2]) : server_port;
        
        c = connection(server_port);
        
        response = make_request(c, buf, 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            help();
            return 0;
        }

        if(strcmp(response, "wrong_user_or_password") == 0){
            free(response);
            printf("wrong username or password\n");
            help();
            return 0;
        }

        if(strcmp(response, "ok") != 0){
            free(response);
            printf("request error\n");
            help();
            return 0;
        }

        /* set current username */
        current_username = malloc(sizeof(params[0]) + sizeof(char));
        strcpy(current_username, params[0]);

        printf("congratulations {%s}, you're logged in!\n", params[0]);

        user_create_folder(current_username);

        clear_out_time(params[0]);
        help();
        return 0;

    }

    /* out */

    if(strcmp(command, "out") == 0){

        if(!logged()){
            close_all_connections();
            return 1;
        }

        /* if logged check if the server is online otherwise save out time */

        c = connection(server_port);

        if(c == NULL){
            printf("saving out time...\n");
            save_out_time(current_username);
        }

        close_all_connections();
        return 1;
    }
    
    if(!logged()){
        printf("you must do the login before doing any action beside login, signup or out\n");
        return 0;
    }

    /* hanging */

    if(strcmp(command, "hanging") == 0){
        
        if(len != 0){
            printf("error wrong format, type:\n\nhanging\n\n");
            return 0;
        }

        c = connection(server_port);
        
        response = make_request(c, "hanging", 1);

        if(response == NULL){
            printf("error connectiong to the server\n");
            help();
            return 0;
        }

        if(strlen(response) == 0){
            printf("no message pending...\n");
            help();
            return 0;
        }
        
        printf("%s\n", response);
        return 0;
    }

    /* show username*/

    if(strcmp(command, "show") == 0){
        
        if(len != 1){
            printf("error wrong format, type:\n\nshow username\n\n");
            help();
            return 0;
        }

        if(strcmp(params[0], current_username) == 0){
            printf("can't show messages from you\n");
            help();
            return 0;
        }

        c = connection(server_port);
        
        sprintf(buf, "show|%s", params[0]);
        
        response = make_request(c, buf, 1);
        
        if(response == NULL){
            printf("error connectiong to the server\n");
            help();
            return 0;
        }

        if(strcmp(response, ERROR_MESSAGE) == 0){
            printf("sorry the username you specified doesn't exist\n");
            help();
            return 0;
        }

        if(strlen(response) == 0){
            printf("no message to show\n");
            help();
            return 0;
        }

        message = strtok(response, "\n");

        while (message){
            printf("%s := %s\n", params[0], message);
            user_received_message(params[0], message, get_current_time());
            message = strtok(NULL, "\n");
        }
        help();
        return 0;
    }

    /* chat username */

    if(strcmp(command, "chat") ==  0){

        if(len != 1){
            printf("error wrong format, type:\n\nchat username\n\n");
            help();
            return 0;
        }
        
        if(strcmp(current_username, params[0]) ==  0){
            printf("error can't chat with yourself\n");
            help();
            return 0;
        }
        
        if(!is_in_contacts(params[0])){
            printf("error %s is not in your contacts\n", params[0]);
            help();
            return 0;
        }
        /* if server is online check if there are some buffered read messaged notification */

        c = connection(server_port);
        
        sprintf(buf, "get_has_read|%s", params[0]);

        make_request(c, buf, 0);

        /* open the chat anyway */

        add_to_chat(params[0], NULL, 1);
        
        return 0;
    }

    /* share file_path */

    if(strcmp(command, "share") == 0){
        
        printf("open a chat with someone to share a file...\n");
        help();
        return 0;
    }

    
    printf("sorry %s is not a valid command\n", command);
    help();
    return 0;
}

char * get_request(char * request, char ** params, int len, int sd, char * raw){
    
    time_t t;
    connection_data * c;
    int rejoin = 0;

    if(request == NULL)
        return NULL;

    if(strcmp(request, "message") == 0){

        if(len != 4) return NULL;

        sscanf(params[3], "%ld", &t);

        user_received_message(params[0], params[2], t);
        refresh_chat();

        /* if you aren't talking to him write a notification */

        if(talking_to_count == 0 || strcmp(params[0], talking_to->username) != 0){
            
            printf("you received a message from {%s}\n", params[0]);
            return NULL;
        }

        /* if you are talking to him */

        refresh_chat();
        return NULL;
    }

    if(strcmp(request, "group_message") == 0){

        if(len != 2) return NULL;

        user_receive_group_message(group_id, params[0], params[1]);
        refresh_chat();
        return NULL;
    }

    if(strcmp(request, "has_read") == 0){

        if(len != 2) return NULL;
        
        sscanf(params[1], "%ld", &t);

        user_has_read(params[0], t);

        if(in_group())
            return NULL;

        if(talking_to_count == 1 && strcmp(talking_to->username, params[0]) == 0)
            refresh_chat();

        return NULL;
    }

    if(strcmp(request, "im") == 0){

        if(len != 2)
            return NULL;

        c = find_connection_by_sd(sd);
        connection_set_username(c->sd, params[0]);
        c->port = atoi(params[1]);

        return NULL;
    }
    
    /* group_add|groud_id|new_member_username|new_meber_port */

    if(strcmp(request, "group_add") == 0){
                
        if(len != 3) return NULL;

        sscanf(params[0],"%ld", &t);
        
        /* if you weren't in the group */

        if(!in_group() || group_id != t){

            /* if you were in another group */    
        
            rejoin = in_group() && group_id != t;

            /* close the chat */

            clear_chatters();

            /* set new group_id */

            group_id = t;

            /* add the group creator to the chat */
            
            c = find_connection_by_sd(sd);
            add_to_chat(c->username, c, 0);

            /* if you left the chat with him cause you were in another group rejoin it */

            if(rejoin){
                
                sprintf(buf, "group_member|%ld|%s|%d", group_id, current_username, current_port);
                printf("to rejoin := %s\n", buf);
                make_request(c, buf, 0);
            }
        }
        
        /* check if you have a connection with the new_mermber otherwise create it */

        c = find_connection_by_username(params[1]);

        if(c == NULL){
            c = connection(atoi(params[2]));
            connection_set_username(c->sd, params[1]);
        }

        c->port = atoi(params[2]);

        add_to_chat(params[1], c, 0);

        /* tell the new member who you are */

        sprintf(buf, "group_member|%ld|%s|%d", group_id, current_username, current_port);
        make_request(c, buf, 0);
        return NULL;
    
    }

    if(strcmp(request, "group_member") == 0){
        if(len != 3) return NULL;

        sscanf(params[0], "%ld", &t);

        /* if you've been added to a new group leave the current chat */

        if(!in_group() || t != group_id){
            clear_chatters();            
            group_id = t;
        }  

        /* get the group partecipant data */

        c = find_connection_by_sd(sd);
        connection_set_username(c->sd, params[1]);
        c->port = atoi(params[2]);

        if(already_talking_to(params[1]))
            return NULL;

        /* add him to chat */

        add_to_chat(params[1], c, 0);

        return NULL;
    }


    if(strcmp(request, "group_quit") == 0){
        if(len != 0)
            return NULL;

        if(!in_group())
            return NULL;

        group_quit(find_connection_by_sd(sd));

        return NULL;
    }

    if(strcmp(request, "share") == 0){
        
        if(len != 1)
            return NULL;
        
        c = find_connection_by_sd(sd);
        printf("{%s} is sending you a file called %s\n", c->username, params[0]);

        receive_file(c, params[0]);

        printf("received!\n");
        return NULL;
    }

    if(strcmp(request, "connected_on_another_device") == 0){
        
        if(len != 0)
            return NULL;

        printf("connected on another device...\n");

        close_all_connections();
        exit(0);        
        return NULL;
    }

    return NULL;

}

void disconnected(int sd){

    if(in_group()) 
        group_quit(find_connection_by_sd(sd));
}

int main(int argc, char* argv[]){
    
    if(argc != 2){
        printf("error you must specify a port\n");
        exit(1);
    }
    
    current_port = atoi(argv[1]);

    system("clear");
    
    help();

    endpoint(
        current_port, 
        input, 
        get_request, 
        disconnected,
        0
    );

    return 1;
}