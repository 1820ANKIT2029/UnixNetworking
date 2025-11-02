#ifndef ANKTI_TCP_CONNECT
#define ANKTI_TCP_CONNECT

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "a_error.h"

int tcp_connect(
    const char *hostname, 
    const char *serv
);

#endif