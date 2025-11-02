#include "udp_connect.h"

int udp_connect(
    const char *hostname,
    const char *serv
) {
    int sockfd;
    struct addrinfo hints, *res, *ressave;
    int n;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if( (n = getaddrinfo(hostname, serv, &hints, &res)) != 0) {
        err_quit("udp_client error for %s, %s: %s", hostname, serv, gai_strerror(n));
    }

    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd >= 0) break;

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;

        close(sockfd);
    } while ( (res = res->ai_next) != NULL);

    if(res == NULL) 
        err_sys("udp_client error for %s, %s", hostname, serv);

    freeaddrinfo(ressave);

    return sockfd;
}