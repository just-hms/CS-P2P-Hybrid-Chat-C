#include "connection.h"

connection_data * head = NULL;

int count = 0;

connection_data * add_connection(int sd, int port){
    
    connection_data * new_connection;
    
    new_connection = (connection_data *) malloc(sizeof(connection_data));
    new_connection->port = port;
    new_connection->sd = sd;

    if(head == NULL){
        
        head = new_connection;
        head->next = NULL;
        count++;
        return new_connection;
    }

    new_connection->next = head;
    head = new_connection;
    count++;
    return new_connection;
}

void remove_connection(int sd){

    connection_data * last;
    connection_data * cursor;
    
    if(head == NULL)
        return;
    
    cursor = head;
    last = NULL;

    if(head->sd == sd){

        head = head->next; 
        free(cursor);
        cursor = NULL;
        count--;
        return;
    }
    
    while (cursor){
        
        if(cursor->sd == sd){
            last->next = cursor->next;
            free(cursor);
            cursor = NULL;
            count--;
            return;
        }

        last = cursor;
        cursor = cursor->next;
    }

}

connection_data * connection(int port){
    
    int res, sd;
    struct sockaddr_in srv_addr;
    connection_data * c;
    
    c = find_connection_by_port(port);
    
    if(c != NULL){
        return c;
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    res = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    
    if(res < 0)
        return NULL;

    add_new_connection(sd);

    return add_connection(sd, port);
}

char * make_request(connection_data * connection, char * request, int need_response){
    
    int res;
    char * buf;

    if(connection == NULL)
        return NULL;

    res = send_message(connection->sd, request);

    if(res <= 0){
        remove_connection(connection->sd);
        return NULL;
    }

    if(need_response){
        
        buf = (char *) malloc(BUF_LEN);
        
        res = receive_message(connection->sd, buf);
        
        if(res <= 0){
            remove_connection(connection->sd);
            free(buf);
            return NULL;
        }

        return buf;
    }
    
    return NULL;
}

connection_data * find_connection_by_port(int port){

    connection_data * cursor;

    cursor = head;

    while (cursor){
        
        if(cursor->port == port)
            return cursor;

        cursor = cursor->next;
    }

    return NULL;
}

void close_all_connections(){
    connection_data * to_remove;

    while (head){
        to_remove = head;
        head = head->next;
        free(to_remove);
    }
    count = 0;
}

int count_connections(){
    return count;
}

connection_data * find_connection_by_username(char * username){

    connection_data * cursor;

    cursor = head;

    while (cursor){
        
        if(cursor->username == username)
            return cursor;

        cursor = cursor->next;
    }

    return NULL;
}
