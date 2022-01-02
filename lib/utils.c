
#include "utils.h"

int receive_message(int i, char * buf){

    int res;

    res = recv(i, (void*)buf, BUF_LEN, 0);
    
    if(res < 0){
        buf = NULL;
        return -1;
    }

    buf[BUF_LEN - 1] = '\0'; 
    return res;
}

int send_message(int i, char * message){
    
    int len, res;
    char buf[BUF_LEN];
    
    len = strnlen(message, BUF_LEN) + 1;
    
    if(len >= BUF_LEN)
        return -1;
    
    strcpy(buf, message);

    buf[len] = '\0';

    res = send(i, (void*) buf, BUF_LEN, 0);
    
    return res;
}

/* FIX ME strlen not secure */

int starts_with(char * first, char * second){
    int len_first = strlen(first);
    int len_second = strlen(second);

    if(len_first > len_second)
        return strncmp(first, second, len_second) == 0;
    
    return strncmp(first, second, len_first) == 0;
}

char * build_string(char * s){
    char * res;
    res = malloc(sizeof(char) * (strlen(s) + 1));
    strcpy(res, s);
    return res;
}


/* testing */

void params_test(char * command, char ** params, int len){
    int i;
    printf("command := {%s}\n", command);

    for (i = 0; i < len; i++){
        printf("param[%i] := {%s}\n", i, params[i]);
    }
}

char * replace_n_with_0(char * string){
    int i;
    int len;
    len = strlen(string);
    for (i = 0; i < len; i++){
        if(string[i] == '\n'){
            string[i] = '\0';
            return string;
        }
        if(string[i] == '\0')
            return string;
    }
    
}