#ifndef ANKIT_HOST_SERV_INFO
#define ANKIT_HOST_SERV_INFO

#define _POSIX_C_SOURCE 200809L     // host_serv_info is behind some macro, adding this enable field visiblilty and doc suggestion

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct addrinfo * host_serv_info(
    const char *hostname, 
    const char *service,
    int family,
    int socktype
);

#endif