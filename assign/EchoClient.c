// EchoClient.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(server_addr);

    if (strlen(argv[2]) < 1 || strlen(argv[2]) > 32) {
        fprintf(stderr, "Error: Message length must be between 1 and 32 bytes.\n");
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    sendto(sockfd, argv[2], strlen(argv[2]), 0,
           (struct sockaddr*)&server_addr, addr_len);
    printf("Sent to server: %s\n", argv[2]);

    int n = recvfrom(sockfd, buffer, BUF_SIZE, 0,
                     (struct sockaddr*)&server_addr, &addr_len);
    buffer[n] = '\0';
    printf("Received from server: %s\n", buffer);

    close(sockfd);
    return 0;
}
