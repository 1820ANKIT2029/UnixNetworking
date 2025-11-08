// gcc -I../lib unixbind.c ../lib/a_error.c -o unixbind-test
// ./unixbind-test /tmp/ankit   

#include "a_error.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

// #define SUN_LEN(ptr) ((size_t)(((struct sockaddr_un *)0)->sun_path) \
//                       + strlen((ptr)->sun_path))

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_un addr1, addr2;
    socklen_t len;

    if(argc != 2) {
        err_quit("usage: unixbind <pathname>");
    }

    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);

    unlink(argv[1]);

    memset(&addr1, 0, sizeof(struct sockaddr_un));
    addr1.sun_family = AF_LOCAL;
    strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1);
    bind(sockfd, (struct sockaddr *) &addr1, SUN_LEN(&addr1));

    len = sizeof(addr2);
    getsockname(sockfd, (struct sockaddr *) &addr2, &len);
    printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);
}