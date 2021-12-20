#include "./../lib/server.c"

void input(char * input){
    printf("%s", input);
}

char * get_request(char * request){
    printf("%s", request);
}

int main(int argc, char* argv[]){
    server(4040, input, get_request);
}