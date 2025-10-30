#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char buf[BUFSIZ];      // buffer

void doit(int);

int main(int argc, char **argv) {
    int sock_fd;
    struct sockaddr_in serv_addr;

    if(argc < 3) {
        printf("usage: %s <IP-Addr> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1) {
        perror("error in socket init\n");
        exit(EXIT_FAILURE);
    }

    if(connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
        perror("error in connect process\n");
        exit(EXIT_FAILURE);
    }

    doit(sock_fd);

    return 0;
}

void doit(int sock_fd) {
    int n;
    while(fgets(buf, BUFSIZ, stdin) != NULL){
        write(sock_fd, buf, strlen(buf));

        n = read(sock_fd, buf, BUFSIZ);
        if(strncmp(buf, "exit", 4) == 0){
            return;
        }

        fputs(buf, stdout);        
    }
}