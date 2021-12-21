#include "utils.h"
#include "connection.h"

int client(char* server_ip, int server_port, char * username){
    
    int ret, sd;
    struct sockaddr_in srv_addr;
    
    sd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &srv_addr.sin_addr);
    
    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    if(ret < 0)
        return -1;
    
    add_connection(sd, server_port);

    return sd;
}

char * request(int sd, char * request, int * ret, int need_response){
    
    char * buffer;

    *ret = send_message(sd, request);

    if(*ret <= 0)
        return NULL;

    if(need_response){
        
        buffer = (char *) malloc(BUF_LEN);
        
        *ret = receive_message(sd, buffer);
        
        if(*ret <= 0){
            free(buffer);
            return NULL;

        }

        return buffer;
    }
    
    return NULL;
}
