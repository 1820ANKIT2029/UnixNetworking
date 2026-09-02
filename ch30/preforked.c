// gcc -o preforked-test preforked.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define NCHILDREN 5
#define BUFSIZE 1024

// Shared structure placed in mapped shared memory
typedef struct {
    pthread_mutex_t mutex;
    long count[NCHILDREN]; // request count for each child
} SharedData;

static SharedData *shared_data;
static pid_t parent_pid;
static pid_t pids[NCHILDREN];

// Initialize Process-Shared Mutex
void mutex_init(SharedData *sd) {
    pthread_mutexattr_t mattr;

    pthread_mutexattr_init(&mattr);
    // IMPORTANT: Set process-shared attribute so mutex works across fork()
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&sd->mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);
}

// Child process main execution loop
void child_main(int i, int listenfd) {
    int connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    char buf[BUFSIZE];
    char body[128];

    signal(SIGINT, SIG_IGN);

    printf("Child %d (PID %d) starting...\n", i, getpid());

    for (;;) {
        clilen = sizeof(cliaddr);

        pthread_mutex_lock(&shared_data->mutex);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        pthread_mutex_unlock(&shared_data->mutex);

        if (connfd < 0) {
            perror("accept error");
            continue;
        }

        shared_data->count[i]++;

        int body_len = snprintf(body, sizeof(body), "Handled by Child %d\n", i);
        int len = snprintf(
            buf, sizeof(buf),
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            body_len, body
        );

        write(connfd, buf, len);
        close(connfd);
    }
}

// Signal handler for graceful termination and stats printing
void sig_int(int signo) {
    if (getpid() != parent_pid) return;

    printf("\n--- Child Process Handling Statistics ---\n");
    for (int i = 0; i < NCHILDREN; i++) {
        printf("Child %d (PID %d) handled %ld connections\n", 
               i, pids[i], shared_data->count[i]);
        kill(pids[i], SIGTERM);
    }
    
    // Destroy shared mutex and unmap memory
    pthread_mutex_destroy(&shared_data->mutex);
    munmap(shared_data, sizeof(SharedData));
    
    while (wait(NULL) > 0); // Wait for children to exit
    printf("Server shut down cleanly.\n");
    exit(0);
}

int main() {
    int listenfd;
    struct sockaddr_in servaddr;

    // Allocate Shared Memory region for Process-Shared Mutex
    shared_data = mmap(
        NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, 
        MAP_SHARED | MAP_ANONYMOUS, -1, 0
    );
    if (shared_data == MAP_FAILED) {
        perror("mmap error");
        exit(1);
    }

    memset(shared_data, 0, sizeof(SharedData));
    mutex_init(shared_data);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) { 
        perror("socket error"); exit(1); 
    }

    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    if (listen(listenfd, 1024) < 0) {
        perror("listen error");
        exit(1);
    }

    parent_pid = getpid();
    signal(SIGINT, sig_int);
    printf(
        "Parent process (PID %d) starting %d preforked children on port %d...\n", 
        parent_pid, NCHILDREN, PORT
    );

    // Prefork Children
    for (int i = 0; i < NCHILDREN; i++) {
        if ((pids[i] = fork()) == 0) {
            child_main(i, listenfd);
        }
    }

    // Parent stays idle waiting for interrupt signal
    for (;;) {
        pause();
    }

    return 0;
}

// for i in {1..5000}; do nc -z localhost 8080 > /dev/null 2>&1; done

/*
Parent process (PID 51106) starting 5 preforked children on port 8080...
Child 0 (PID 51107) starting...
Child 2 (PID 51109) starting...
Child 4 (PID 51111) starting...
Child 1 (PID 51108) starting...
Child 3 (PID 51110) starting...
^C
--- Child Process Handling Statistics ---
Child 0 (PID 51107) handled 1064 connections
Child 1 (PID 51108) handled 986 connections
Child 2 (PID 51109) handled 922 connections
Child 3 (PID 51110) handled 893 connections
Child 4 (PID 51111) handled 1135 connections
Server shut down cleanly.
*/