#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

char rec_buf[BUFSIZ], send_buf[BUFSIZ];      // buffer

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

    shutdown(sock_fd, SHUT_WR);
    return 0;
}

// using select 
void doit(int sock_fd) {
    int n;
    int maxfd1;
    fd_set rset; // read fd_set

    FD_ZERO(&rset);
    while(1){
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sock_fd, &rset);
        maxfd1 = MAX(STDIN_FILENO, sock_fd) + 1;
        select(maxfd1, &rset, NULL, NULL, NULL);  // I/O multiplexing on read fds

        // STDIN ready for I/O
        if(FD_ISSET(STDIN_FILENO, &rset)) {
            if( (n = read(STDIN_FILENO, send_buf, BUFSIZ)) == 0) {
                return;
            }
            write(sock_fd, send_buf, n);
        }

        // sock_fd (client socket) ready for I/O
        if(FD_ISSET(sock_fd, &rset)) {
            n = read(sock_fd, rec_buf, BUFSIZ);
            if(strncmp(rec_buf, "exit", 4) == 0 || n == 0){
                return;
            }
            write(STDOUT_FILENO, rec_buf, n);
        }
    }
}