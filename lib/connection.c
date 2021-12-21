#include "connection.h"


connection * head = NULL;

int count = 0;

void add_connection(int sd, int port){
    connection * new_connection;
    
    new_connection = (connection *) malloc(sizeof(connection));
    new_connection->port = port;
    new_connection->sd = sd;

    if(head == NULL){
        head = new_connection;
        head->next = NULL;
        count++;
        return;
    }

    new_connection->next = head;
    head = new_connection;
    count++;
}

void remove_connection(int sd){
    connection * last;
    connection * cursor;
    
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

connection * find_connection(int sd){
    connection * cursor;

    cursor = head;

    while (cursor){
        
        if(cursor->sd = sd)
            return cursor;

        cursor = cursor->next;
    }

    return NULL;
}

int count_connections(){
    return count;
}