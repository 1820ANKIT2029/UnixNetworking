// gcc -I../lib daytime_udp_server.c ../lib/a_error.c ../lib/udp_server.c -o daytime_udp_server-test

#include "udp_server.h"
#include <time.h>
#include <arpa/inet.h>

#define MAXLINE 1024

char *sock_ntop(const struct sockaddr *sa, socklen_t salen);

int main(int argc, char **argv) {
    int sockfd, n;
    char buf[MAXLINE + 1];
    time_t ticks;
    struct sockaddr_storage cliaddr;
    socklen_t len;

    if(argc == 2 ) {
        sockfd = udp_server(NULL, argv[1], NULL);
    }
    else if (argc == 3) {
        sockfd = udp_server(argv[1], argv[2], NULL);
    }
    else {
        err_quit("usage: %s [<hostname>] <service or port#>", argv[0]);
    }

    while(1) {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
        printf("datagram from %s\n", sock_ntop((struct sockaddr *) &cliaddr, len));

        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &cliaddr, len);
    }
}

char *sock_ntop(const struct sockaddr *sa, socklen_t salen){
    char portstr[7];
    static char str[128];

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}