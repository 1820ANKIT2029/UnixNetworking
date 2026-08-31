#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define BROADCAST_IP "255.255.255.255"

int main() {
    int sockfd;
    struct sockaddr_in broadcast_addr;
    int broadcast_enable = 1;
    const char *message = "Hello, Broadcast Network!";

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt(SO_BROADCAST) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, BROADCAST_IP, &broadcast_addr.sin_addr);

    ssize_t sent_bytes = sendto(sockfd, message, strlen(message), 0,
                               (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
    if (sent_bytes < 0)
        perror("sendto failed");
    else
        printf("Successfully sent %zd broadcast bytes to %s:%d\n", sent_bytes, BROADCAST_IP, PORT);

    close(sockfd);
    return 0;
}