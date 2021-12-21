#include "utils.h"
#include "connection.h"

int client(char* server_ip, int server_port){
    
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

char * request(int sd, char * request, int need_response){
    int ret;
    char * buffer;

    ret = send_message(sd, request);

    if(need_response){
        
        buffer = (char *) malloc(BUF_LEN);
        
        receive_message(sd, buffer);
        return buffer;
    }
    
    return NULL;
}
