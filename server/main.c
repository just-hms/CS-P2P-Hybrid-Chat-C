#include "./../lib/endpoint.h"
#include "./../lib/utils.h"

/* TODO io.h ???*/

#include "./../lib/io.h"
/*
    Digita un comando:
    1) help --> mostra i dettagli dei comandi
    2) list --> mostra un elenco degli utenti connessi
    3) esc --> chiude il
*/
void help(){
    printf("\thelp --> show commands details\n");
    printf("\tlist --> show user list \n");
    printf("\tesc --> shut down the server\n");

    printf("\noptionals:\n\n");
    printf("\tcls --> clear the screen\n\n");
}
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
        help();
        return;
    }

    if(strcmp(command, "cls") == 0){
        system("clear");
        return;
    }

    printf("sorry %s is not a valid command\n", command);

}

char * get_request(char * request, char ** params, int len){
    
    int res;
    
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