#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

int SERV_PORT = 8000;
char buf[BUFSIZ];

int main() {
    int serv_fd;
    struct sockaddr_in serv_addr, client_addr;
    int client_addr_len, n;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    serv_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serv_fd == -1) {
        printf("error in socket init\n");
        exit(1);
    }

    int b_code = bind(serv_fd, (struct sockadrr *) &serv_addr, sizeof(serv_addr));
    if(b_code == -1) {
        printf("error in bind process\n");
        exit(1);
    }

    while(1) {
        client_addr_len = sizeof(client_addr);
        n = recvfrom(serv_fd, buf, BUFSIZ, 0, (struct sockadrr *) &client_addr, &client_addr_len);
        buf[n] = '\0';
        printf("data recv: %s\n", buf);
        sendto(serv_fd, buf, n, 0, (struct sockadrr *) &client_addr, client_addr_len);
    }

}