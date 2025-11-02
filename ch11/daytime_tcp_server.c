// gcc -I../lib daytime_tcp_server.c ../lib/a_error.c ../lib/tcp_listen.c -o daytime_tcp_server-test
// sudo ./daytime_tcp_server-test daytime

#include "tcp_listen.h"
#include <time.h>
#include <arpa/inet.h>

#define MAXLINE 1024

char *sock_ntop(const struct sockaddr *sa, socklen_t salen);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t len;
    char buf[MAXLINE + 1];
    time_t ticks;
    struct sockaddr_storage cliaddr;

    if(argc != 2 ) {
        err_quit("usage: %s <service or port#>", argv[0]);
    }

    listenfd = tcp_listen(NULL, argv[1], NULL);

    while(1) {
        len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
        printf("connection from %s\n", sock_ntop((struct sockaddr *) &cliaddr, len));

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