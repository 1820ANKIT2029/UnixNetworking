// tcp_reverse_server.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

void reverse_string(char *str) {
    int i, len = strlen(str);
    for (i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUF_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 10);
    printf("Server listening on port %d...\n", PORT);

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        printf("Client connected: %s\n", inet_ntoa(address.sin_addr));

        while (1) {
            memset(buffer, 0, BUF_SIZE);
            int bytes = read(new_socket, buffer, BUF_SIZE);
            if (bytes <= 0) break;

            printf("Received: %s\n", buffer);

            reverse_string(buffer);
            printf("Sending: %s\n", buffer);

            send(new_socket, buffer, strlen(buffer), 0);
        }

        close(new_socket);
    }
    
    close(server_fd);
    return 0;
}
