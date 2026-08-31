#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAXLINE 4096

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void str_cli(FILE *fp, int sockfd) {
    int maxfdp1, val, stdineof;
    ssize_t n, nwritten;
    fd_set rset, wset;
    
    char to[MAXLINE], fr[MAXLINE];
    char *toiptr, *tooptr, *friptr, *froptr;

    int stdin_fd = fileno(fp);
    int stdout_fd = fileno(stdout);

    // Set all three descriptors to nonblocking mode
    set_nonblocking(sockfd);
    set_nonblocking(stdin_fd);
    set_nonblocking(stdout_fd);

    toiptr = tooptr = to;
    friptr = froptr = fr;
    stdineof = 0;

    maxfdp1 = MAX(MAX(stdin_fd, stdout_fd), sockfd) + 1;

    for (;;) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);

        // Read from stdin if buffer has space and stdin is not at EOF
        if (stdineof == 0 && toiptr < &to[MAXLINE])
            FD_SET(stdin_fd, &rset);

        // Read from socket if 'fr' buffer has space
        if (friptr < &fr[MAXLINE])
            FD_SET(sockfd, &rset);

        // Write to socket if 'to' buffer contains unwritten data
        if (tooptr != toiptr)
            FD_SET(sockfd, &wset);

        // Write to stdout if 'fr' buffer contains unwritten data 
        if (froptr != friptr)
            FD_SET(stdout_fd, &wset);

        if (select(maxfdp1, &rset, &wset, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select error");
            exit(1);
        }

        // Handle stdin input
        if (FD_ISSET(stdin_fd, &rset)) {
            if ((n = read(stdin_fd, toiptr, &to[MAXLINE] - toiptr)) < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("read error on stdin");
                    exit(1);
                }
            } 
            else if (n == 0) {
                stdineof = 1;               // EOF on stdin 
                if (tooptr == toiptr) shutdown(sockfd, SHUT_WR); // Send FIN to server
            } 
            else {
                toiptr += n;                // Advance input pointer
                FD_SET(sockfd, &wset);      // Try to write to socket immediately
            }
        }

        // Handle socket input
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("read error on socket");
                    exit(1);
                }
            } 
            else if (n == 0) {
                if (stdineof) return;                 // Normal termination
                else {
                    fprintf(stderr, "str_cli: server terminated prematurely\n");
                    exit(1);
                }
            } 
            else {
                friptr += n;                // Advance input pointer
                FD_SET(stdout_fd, &wset);   // Try to write to stdout immediately
            }
        }

        // Handle stdout output
        if (FD_ISSET(stdout_fd, &wset)) {
            if ((nwritten = &fr[MAXLINE] - froptr) > (friptr - froptr))
                nwritten = friptr - froptr;

            if ((n = write(stdout_fd, froptr, nwritten)) < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("write error to stdout");
                    exit(1);
                }
            } 
            else {
                froptr += n;                // Advance output pointer
                if (froptr == friptr)
                    froptr = friptr = fr;   // Reset pointers when buffer is empty
            }
        }

        // Handle socket output 
        if (FD_ISSET(sockfd, &wset)) {
            if ((nwritten = &to[MAXLINE] - tooptr) > (toiptr - tooptr))
                nwritten = toiptr - tooptr;

            if ((n = write(sockfd, tooptr, nwritten)) < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("write error to socket");
                    exit(1);
                }
            } 
            else {
                tooptr += n;                // Advance output pointer
                if (tooptr == toiptr) {
                    toiptr = tooptr = to;   // Reset pointers when buffer is empty
                    if (stdineof)
                        shutdown(sockfd, SHUT_WR); // Send FIN if EOF reached
                }
            }
        }
    }
}