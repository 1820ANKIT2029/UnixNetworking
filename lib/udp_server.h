#ifndef ANKIT_UDP_SERVER
#define ANKIT_UDP_SERVER

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "a_error.h"

int udp_server(
    const char *hostname,
    const char *serv,
    socklen_t *addrlenp
);

#endif