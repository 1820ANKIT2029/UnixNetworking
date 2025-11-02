#ifndef ANKIT_TCP_LISTEN
#define ANKIT_TCP_LISTEN

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "a_error.h"

#define LISTENQ 100

int tcp_listen(
    const char *hostname, 
    const char *serv,
    socklen_t *addrlenp
);

#endif