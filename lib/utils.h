
#define BUF_LEN 1024

struct connections{
    int isClient;
    struct sockaddr_in cl_addr;
    int fd;
};

void receive_message(int, char *);
int send_message(int, char*);
