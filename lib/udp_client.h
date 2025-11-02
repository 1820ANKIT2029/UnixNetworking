#ifndef ANKIT_UDP_CLIENT
#define ANKIT_UDP_CLIENT

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "a_error.h"

int udp_client(
    const char *hostname,
    const char *serv,
    struct sockaddr  **saptr,
    socklen_t *lenp
);

#endif