#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <poll.h>
#include <errno.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int SERV_PORT = 8000;  // port
char buf[BUFSIZ];      // buffer

void sig_chld(int);

int main() {
    int nready, maxi;
    struct pollfd client[FOPEN_MAX];
    int serv_fd, client_fd, tmp_fd;
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

    if(listen(serv_fd, 100) == -1) {
        perror("error in listen process\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening of port %d\n", SERV_PORT);

    // server fd added for listen to poll struct
    client[0].fd = serv_fd;
    client[0].events = POLLIN;

    for(int i=1; i<FOPEN_MAX; i++) client[i].fd = -1;
    maxi = 0;

    while(1){
        nready = poll(client, maxi + 1, -1);

        // accept new connection
        if(client[0].revents & POLLIN){
            client_addr_len = sizeof(client_addr);
            client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_addr_len);
            int i;
            for(i=1; i<FOPEN_MAX; i++) {
                if(client[i].fd < 0) {
                    client[i].fd = client_fd;
                    client[i].events = POLLIN;
                    printf("Client connected: {%d}\n", i);
                    break;
                }
            }
            if(i == FOPEN_MAX) {
                printf("Too many clients!\n");
                close(client_fd);
                goto client_overflow;
            }

            maxi = MAX(maxi, i);

            client_overflow:
            if(--nready <= 0) continue;
        }
    
        
        // client connection handler
        int i, n;
        for(i=1; i <= maxi; i++) {
            if( (tmp_fd = client[i].fd) < 0) continue;     // no client at client[i]

            if(client[i].revents & (POLLIN | POLLERR)) {
                n = read(tmp_fd, buf, BUFSIZ);
                if(n < 0) {
                    if(errno == ECONNRESET) {
                        close(tmp_fd);
                        printf("Closing client connection: {%d}\n", i);
                        client[i].fd = -1;
                    }
                    else {
                        printf("Error in read: {i}\n");
                    }
                }
                else if (n == 0) {
                    close(tmp_fd);
                    printf("Closing client connection: {%d}\n", i);
                    client[i].fd = -1;
                }
                else {
                    write(tmp_fd, buf, n);
                }

                if(--nready <= 0) break;
            }
        }
    }

    close(serv_fd);
    return 0;
}