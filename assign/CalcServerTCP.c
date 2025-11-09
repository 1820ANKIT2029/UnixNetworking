// CalcServerTCP.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define BUF_SIZE 1024

double evaluate(char *expr) {
    double a, b;
    char op;
    sscanf(expr, "%lf %c %lf", &a, &op, &b);
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : NAN;
        case '^': return pow(a, b);
        default: return NAN;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUF_SIZE];
    int port = atoi(argv[1]);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);
    printf("Server listening on port %d\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        printf("Client connected: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        while (1) {
            memset(buffer, 0, BUF_SIZE);
            int bytes = read(client_fd, buffer, BUF_SIZE);
            if (bytes <= 0) break;
            if (strncmp(buffer, "-1", 2) == 0) break;

            printf("Received: %s\n", buffer);
            double result = evaluate(buffer);

            sprintf(buffer, "%.2f", result);
            send(client_fd, buffer, strlen(buffer), 0);
            printf("Sent result: %s\n", buffer);
        }
        close(client_fd);
        printf("Client disconnected.\n");
    }
}
