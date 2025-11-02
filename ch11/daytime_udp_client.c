// gcc -I../lib daytime_udp_client.c ../lib/a_error.c ../lib/udp_client.c -o daytime_udp_client-test

#include "udp_client.h"
#include <arpa/inet.h>

#define BUFLINE 1024

char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen);

int main(int argc, char **argv) {
    int sockfd;
    char buf[BUFLINE];
    int n;
    struct sockaddr *sa;
    socklen_t salen;
    
    if(argc != 3) {
        err_quit("usage: %s <hostname/IPaddress> <service/port#>", argv[0]);
    }

    sockfd = udp_client(argv[1], argv[2], (struct sockaddr **) &sa, &salen);

    printf("sending to %s\n", sock_ntop_host(sa, salen));
    
    sendto(sockfd, "", 1, 0, sa, salen);
    n = recvfrom(sockfd, buf, BUFLINE, 0, NULL, NULL);
    buf[n] = 0;
    fputs(buf, stdout);

    exit(0);
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