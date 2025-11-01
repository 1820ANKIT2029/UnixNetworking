#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("usage: %s <hostname> <service>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in serv_addr;
    int serv_addr_len, n;
    char buf[BUFSIZE];
    struct in_addr **pptr;
    struct in_addr *inetaddrp[2];
    struct in_addr inetaddr;
    struct hostent *host_ip_info;
    struct servent *service_info;
    char buf4[INET_ADDRSTRLEN];

    if( (host_ip_info = gethostbyname(argv[1])) == NULL) {
        if(inet_aton(argv[1], &inetaddr) == 0) {
            printf("hostname error for %s: %s\n", argv[1], hstrerror(h_errno));
            exit(EXIT_FAILURE);
        }
        else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }
    }
    else {
        pptr = (struct in_addr **) host_ip_info->h_addr_list;
    }

    if( (service_info = getservbyname(argv[2], "tcp")) == NULL) {
        printf("getservbyname error for %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    for(; *pptr != NULL; pptr++) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = service_info->s_port;
        memcpy(&serv_addr.sin_addr, *pptr, sizeof(struct in_addr));

        printf("trying %s:%d\n", inet_ntop(AF_INET, *pptr, buf4, sizeof(buf4)), ntohs(service_info->s_port));
        if( connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0)
            break;

        close(sock_fd);
    }

    if(*pptr == NULL) {
        printf("unable to connect\n");
        exit(EXIT_FAILURE);
    }

    printf("connected %s:%d\n", inet_ntop(AF_INET, *pptr, buf4, sizeof(buf4)), ntohs(service_info->s_port));
    while( (n = read(sock_fd, buf, BUFSIZE)) > 0) {
        buf[n] = 0;
        fputs(buf, stdout);
    }

    close(sock_fd);
}