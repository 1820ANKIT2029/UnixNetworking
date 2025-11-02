#include "udp_client.h"

int udp_client(
    const char *hostname,
    const char *serv,
    struct sockaddr  **saptr,
    socklen_t *lenp
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
    } while ( (res = res->ai_next) != NULL);

    if(res == NULL) 
        err_sys("udp_client error for %s, %s", hostname, serv);

    *saptr = malloc(res->ai_addrlen);
    memcpy(*saptr, res->ai_addr, res->ai_addrlen);
    *lenp = res->ai_addrlen;

    freeaddrinfo(ressave);

    return sockfd;
}