// gcc -I../lib unixstrserv01.c ../lib/a_error.c -o unixstrserv01-test
// ./unixstrserv01-test

#include <stdio.h>
#include "a_error.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/signal.h>
#include <sys/wait.h>

#define UNIXSTR_PATH "/tmp/ankit"
#define MAXLINE 1024

void str_echo(int sockfd);
void sig_chld(int signo);

int main(int argc, char **argv){
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_un	cliaddr, servaddr;
	void				sig_chld(int);

	listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);

	unlink(UNIXSTR_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSTR_PATH);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, 100);

	signal(SIGCHLD, sig_chld);

	while(1){
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;
			else
				err_sys("accept error");
		}

		if ( (childpid = fork()) == 0) {	/* child process */
			close(listenfd);	/* close listening socket */
			str_echo(connfd);	/* process the request */
			exit(0);
		}
		close(connfd);			/* parent closes connected socket */
	}
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    // Clean up all terminated children
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("Child %d terminated\n", pid);
    }
}

void str_echo(int sockfd) {
	ssize_t	n;
	char line[MAXLINE];
	
	while(1) {
		if ( (n = read(sockfd, line, MAXLINE)) == 0)
			return;		/* connection closed by other end */

		write(sockfd, line, n);
	}
}