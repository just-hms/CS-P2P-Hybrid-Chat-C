#include "./../lib/server.c"
#include "../lib/utils.h"

void input(char * input){

    if(starts_with(input, "signup")){
        
        return;
    }
    if(starts_with(input, "in")){
        
        return;
    }
    if(starts_with(input, "out")){
        
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

}

char * get_request(char * request){
    
}

int main(int argc, char* argv[]){
    // TODO get input for the port listening to
    if(argc == 0){
        printf("ERROR: you must specifie a port to connect to!");
        exit(1);
    }

    // input check
    
    server(
        argv[0], 
        input, 
        get_request, 
        0
    );
}