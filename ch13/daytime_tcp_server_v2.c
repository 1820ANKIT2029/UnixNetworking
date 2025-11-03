// gcc -I../lib daytime_tcp_server_v2.c ../lib/a_error.c ../lib/tcp_listen.c ../lib/daemon_init.c -o daytime_tcp_server_v2-test
// sudo ./daytime_tcp_server_v2-test daytime

#include "daemon_init.h"
#include "tcp_listen.h"
#include <time.h>
#include <arpa/inet.h>

#define MAXLINE 1024

char *sock_ntop(const struct sockaddr *sa, socklen_t salen);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t len, addrlen;
    char buf[MAXLINE + 1];
    time_t ticks;
    struct sockaddr *cliaddr;

    if(argc < 2 || argc > 3) {
        err_quit("usage: %s [host] <service or port#>", argv[0]);
    }

    daemon_init(argv[0], 0);

    if(argc == 2) listenfd = tcp_listen(NULL, argv[1], &addrlen);
    else listenfd = tcp_listen(argv[1], argv[2], &addrlen);

    cliaddr = malloc(addrlen);

    while(1) {
        len = addrlen;
        connfd = accept(listenfd, cliaddr, &len);
        printf("connection from %s\n", sock_ntop(cliaddr, len));

        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));

        close(connfd);
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