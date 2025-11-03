#ifndef ANKIT_DAEMON_INIT
#define ANKIT_DAEMON_INIT

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>

#define MAXFD 64



int daemon_init(const char *pname, int facility);

#endif