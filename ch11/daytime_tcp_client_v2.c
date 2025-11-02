// gcc -I../lib daytime_tcp_client_v2.c ../lib/a_error.c ../lib/tcp_connect.c -o daytime_tcp_client_v2-test
// ./daytime_tcp_client_v2-test time-a-g.nist.gov daytime

#include <arpa/inet.h>

#include "tcp_connect.h"

#define MAXLINE 1024

char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen);

int main(int argc, char **argv) {
    int sockfd;
    char buf[MAXLINE + 1];
    socklen_t len, n;
    struct sockaddr_storage ss;

    if(argc != 3) {
        err_quit("usage: %s <hostname/IPaddress> <service/port#>", argv[0]);
    }

    sockfd = tcp_connect(argv[1], argv[2]);

    len = sizeof(ss);
    getpeername(sockfd, (struct sockaddr *) &ss, &len);
    printf(
        "connected to %s\n", 
        sock_ntop_host((struct sockaddr *) &ss, len)
    );

    while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
        buf[n] = 0;
        fputs (buf, stdout);
    }
    exit (0);
}

char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen){
    static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}
	default:
		snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}

    return (NULL);
}