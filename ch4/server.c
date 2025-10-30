#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

char buf[BUFSIZ];

void doit(int);

// Echo TCP server
// use ncat 127.0.0.1 8000 (client) to see
int main() {
    int serv_fd, client_fd;
    socklen_t client_addr_len;
    struct sockaddr_in serv_addr, client_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8000);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket

    bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(serv_fd, 10);
    printf("Listening of port 8000\n");

    while(1){
        client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        printf("client connected\n");

        // client connection handler
        if(fork() == 0) {
            close(serv_fd);
            doit(client_fd);
            close(client_fd);
            printf("Closing client connection\n");
            exit(EXIT_SUCCESS);
        }

        close(client_fd);
    }

    return 0;
}

void doit(int client_fd){
    int n;
    while(1){
        n = read(client_fd, buf, BUFSIZ);
        if(strncmp(buf, "exit", 4) == 0){
            return;
        }
        write(client_fd, buf, n);
    }
}