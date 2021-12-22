#include "./../lib/endpoint.h"
#include "./../lib/utils.h"

/* TODO io.h ???*/

#include "./../lib/io.c"
/*
    Digita un comando:
    1) help --> mostra i dettagli dei comandi
    2) list --> mostra un elenco degli utenti connessi
    3) esc --> chiude il
*/

void input(char * command, char ** params, int len){
    if(command == NULL){
        return;
    }
    if(strcmp(command, "esc") == 0)
        exit(0);

    if(strcmp(command, "list") == 0){
        /* list users */
        return;
    }

    if(strcmp(command, "help") == 0){
        /* show commands details */
    }

    printf("sorry %s is not a valid command\n", command);

}

char * get_request(char * request, char ** params, int len){
    
    int res;
    
    if(request == NULL)
        return;

    if(strcmp(request, "signup") == 0){
        
        /* TODO testing */
        return build_string("ok");
        
        if(len != 2)
            return build_string("error");

        res = user_add(params[0], params[1]);

        if(res)
            return build_string("ok");

        return build_string("not_available");
    }

    if(strcmp(request, "in") == 0){
        
        if(len != 2)
            return build_string("error");

        res = user_login(params[0], params[1]);

        if(res)
            return build_string("ok");

        return build_string("wrong_user_or_password");
    }
}

/* ./server [port] */

int main(int argc, char* argv[]){
    
    int port;

    port = (argc == 1) ? atoi(argv[0]) : 4040;
    
    endpoint(
        port, 
        input, 
        get_request, 
        0
    );
}