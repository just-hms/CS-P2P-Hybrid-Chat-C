#include "./../lib/client.c"

int main(int argc, char* argv[]){

    int client_id, res;
    char * response;

    client_id = client(4040, "__server");

    if(client_id < 0){
        printf("error connection with the socket\n");
        exit(1);
    }

    response = request(
        client_id, 
        "kek", 
        &res,
        1
    );

    if(res <= 0){
        printf("error during the request\n");
        return;
    }
    
    printf("response received := %s\n", response);

    free(response);
    
}