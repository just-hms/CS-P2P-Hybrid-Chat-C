#include "./../lib/client.c"

int main(int argc, char* argv[]){

    int client_id;

    client_id = client("127.0.0.1", 4040);

    if(client_id < 0){
        perror("errore di connessione al socket\n");
        exit(1);
    }
    while (1){
        request(
            client_id, 
            "kek", 
            0
        );
    }
    
}