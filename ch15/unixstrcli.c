// gcc -I../lib unixstrcli.c ../lib/a_error.c -o unixstrcli-test
// ./unixstrcli-test

#include <unistd.h>
#include "a_error.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define UNIXSTR_PATH "/tmp/ankit"
#define MAXLINE 1024

char buf[MAXLINE];      // buffer

void doit(int sock_fd);

int main(int argc, char **argv){
	int	sockfd;
	struct sockaddr_un servaddr;

	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSTR_PATH);

	connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	doit(sockfd);		/* do it all */

	exit(0);
}

void doit(int sock_fd) {
    int n;
    while(fgets(buf, MAXLINE, stdin) != NULL){
        write(sock_fd, buf, strlen(buf));

        n = read(sock_fd, buf, MAXLINE);
        if(strncmp(buf, "exit", 4) == 0){
            return;
        }

        fputs(buf, stdout);        
    }
}