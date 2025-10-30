#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

int SERV_PORT = 8000;  // port
char buf[BUFSIZ];      // buffer

void doit(int);
void sig_chld(int);

int main() {
    int serv_fd, client_fd;
    socklen_t client_addr_len;
    struct sockaddr_in serv_addr, client_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if(serv_fd == -1) {
        perror("error in socket init\n");
        exit(EXIT_FAILURE);
    }

    int b_code = bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(b_code == -1) {
        perror("error in bind process\n");
        exit(EXIT_FAILURE);
    }

    if(listen(serv_fd, 10) == -1) {
        perror("error in listen process\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening of port %d\n", SERV_PORT);

    signal(SIGCHLD, sig_chld);
    while(1){
        client_addr_len = sizeof(client_addr);
        client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if(client_fd < 0) {
            if(errno == EINTR) continue;
            else {
                perror("error in accept process\n");
                exit(1);
            }
        }
        printf("Client connected\n");

        // client connection handler
        if(fork() == 0) {
            close(serv_fd);
            doit(client_fd);
            printf("Closing client connection\n");
            close(client_fd);
            exit(EXIT_SUCCESS);
        }

        close(client_fd);
    }

    return 0;
}

void doit(int client_fd){
    int n;
    while((n = read(client_fd, buf, BUFSIZ)) > 0){
        if(strncmp(buf, "exit", 4) == 0){
            return;
        }
        write(client_fd, buf, n);
    }
}

void sig_chld(int) {
    pid_t pid;
    int stat;

    while( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);

    return;
}