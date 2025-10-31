#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

char send_buf[BUFSIZ], recv_buf[BUFSIZ];

int main(int argc, char **argv) {
    int client_fd;
    struct sockaddr_in serv_addr;
    int n, serv_addr_len, nready;
    struct pollfd readfd[2];

    if(argc < 3) {
        printf("usage: %s <IP-Addr> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    readfd[0].fd = STDIN_FILENO;
    readfd[0].events = POLLIN;
    readfd[1].fd = client_fd;
    readfd[1].events = POLLIN;

    while(1) {
        nready = poll(readfd, 2, -1);

        if(readfd[0].revents & POLLIN) {
            n = read(STDIN_FILENO, send_buf, BUFSIZ);
            send_buf[n-1] = '\0';
            serv_addr_len = sizeof(serv_addr);
            sendto(client_fd, send_buf, BUFSIZ, 0, (struct sockadrr *) &serv_addr, serv_addr_len);

            if(--nready <= 0) continue;
        }

        if(readfd[1].revents & POLLIN) {
            n = recvfrom(client_fd, recv_buf, BUFSIZ, 0, (struct sockadrr *) &serv_addr, &serv_addr_len);
            printf("%s\n", recv_buf);
        }
    }
}