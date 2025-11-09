// TimeOfDayServer.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Time-of-Day Server started... waiting for requests on port %d...\n", PORT);

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0,
                         (struct sockaddr*)&client_addr, &addr_len);
        buffer[n] = '\0';
        printf("Request received from %s: %s\n", inet_ntoa(client_addr.sin_addr), buffer);

        if (strcasecmp(buffer, "time") == 0) {
            time_t now = time(NULL);
            snprintf(buffer, BUF_SIZE, "Server Time: %s", ctime(&now));
        } else {
            snprintf(buffer, BUF_SIZE, "Invalid request. Send 'time' to get time of day.");
        }

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&client_addr, addr_len);
        printf("Response sent to client.\n");
    }

    close(sockfd);
    return 0;
}
