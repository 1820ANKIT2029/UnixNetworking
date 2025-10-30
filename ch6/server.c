#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int SERV_PORT = 8000;  // port
char buf[BUFSIZ];      // buffer

void sig_chld(int);

int main() {
    int maxfd1, nready;
    fd_set rset, allset;
    int client[FD_SETSIZE], client_size;
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

    if(listen(serv_fd, 10) == -1) {
        perror("error in listen process\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening of port %d\n", SERV_PORT);

    client_size = -1;
    memset(&client, -1, sizeof(client));

    maxfd1 = serv_fd;
    FD_ZERO(&allset);
    FD_SET(serv_fd, &allset);
    while(1){
        rset = allset;
        nready = select(maxfd1 + 1, &rset, NULL, NULL, NULL);

        // accept new connection
        if(FD_ISSET(serv_fd, &rset)){
            client_addr_len = sizeof(client_addr);
            client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_addr_len);
            int i;
            for(i=0; i<FD_SETSIZE; i++) {
                if(client[i] < 0) {
                    client[i] = client_fd;
                    printf("Client connected: {%d}\n", i);
                    break;
                }
            }
            if(i == FD_SETSIZE) {
                printf("Too many clients!\n");
                close(client_fd);
                goto client_overflow;
            }
            FD_SET(client_fd, &allset);
            maxfd1 = MAX(maxfd1, client_fd);
            client_size = MAX(client_size, i);
            if(--nready <= 0) continue;
        }
    
        client_overflow:
        // client connection handler
        int i, n;
        for(i=0; i<=client_size; i++) {
            if( (tmp_fd = client[i]) < 0) continue;     // no client at client[i]

            if(FD_ISSET(tmp_fd, &rset)) {
                if((n = read(tmp_fd, buf, BUFSIZ)) == 0) {
                    close(tmp_fd);
                    printf("Closing client connection: {%d}\n", i);
                    FD_CLR(tmp_fd, &allset);
                    client[i] = -1;
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