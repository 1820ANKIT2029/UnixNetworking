// CalcClientTCP.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    struct hostent *server = gethostbyname(hostname);
    if (!server) {
        fprintf(stderr, "Error: No such host.\n");
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    printf("Connected to %s on port %d\n", hostname, port);

    char buffer[BUF_SIZE];
    while (1) {
        printf("Enter expression (operand1 operator operand2), -1 to quit: ");
        fgets(buffer, BUF_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "-1") == 0) {
            send(sockfd, buffer, strlen(buffer), 0);
            break;
        }

        send(sockfd, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUF_SIZE);
        read(sockfd, buffer, BUF_SIZE);
        printf("Result: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
