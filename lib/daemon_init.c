#include "daemon_init.h"

int daemon_proc;

int daemon_init(const char *pname, int facility) {
    int i;
    pid_t pid;

    if( (pid = fork()) < 0) return -1;
    else if (pid) _exit(0);

    /* child 1 continues... */
    if(setsid() < 0) return -1;

    signal(SIGHUP, SIG_IGN);
    if( (pid = fork()) < 0) return -1;
    else if(pid) _exit(0);

    /* child 2 continues... */
    daemon_proc = 1;

    chdir("/");

    for(int i=0; i<MAXFD; i++) close(i);

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(pname, LOG_PID, facility);

    return 0;
}