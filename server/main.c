#include "./../lib/server.c"

/*
    Digita un comando:
    1) help --> mostra i dettagli dei comandi
    2) list --> mostra un elenco degli utenti connessi
    3) esc --> chiude il
*/

void input(char * input){
    
    if(strcmp(input, "esc") == 0)
        exit(0);

    if(strcmp(input, "list") == 0){
        /* list users */
        return;
    }
    if(strcmp(input, "help") == 0){
        /* show commands details */
    }

}

char * get_request(char * request){
    
    /* ECHO TEST */
    
    char * answer;
    answer = malloc(strlen(request) * sizeof(char) + 1);

    strcpy(answer, request);

    return answer;
}

int main(int argc, char* argv[]){
    server(
        4040, 
        input, 
        get_request, 
        1
    );
}