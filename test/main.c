#include "./../lib/client.c"

int main(int argc, char* argv[]){

    int client_id, ret;
    char * response;

    client_id = client("127.0.0.1", 4040);

    if(client_id < 0){
        printf("error connection with the socket\n");
        exit(1);
    }

    response = request(
        client_id, 
        "kek", 
        &ret,
        1
    );

    if(ret <= 0){
        printf("error during the request\n");
        return;
    }
    
    printf("response received := %s\n", response);

    free(response);
    
}