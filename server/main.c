#include "./../lib/server.c"

void input(char * input){
    printf("%s", input);

}

char * get_request(char * request){
    
    char * asnwer;
    
    asnwer = (char *) malloc(sizeof(char) * 5);
    memset(asnwer, 0, sizeof(char) * 5);

    strcpy(asnwer, "kek\0");

    printf("%s", request);

    return asnwer;
}

int main(int argc, char* argv[]){
    server(4040, input, get_request);
}