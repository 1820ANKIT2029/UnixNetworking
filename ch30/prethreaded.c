// gcc -o prethreaded-test prethreaded.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define NTHREADS 5
#define MAX_QUEUE 500
#define BUFSIZE 1024

// Circular Queue for Descriptor Passing
typedef struct {
    int iget;               // Next index to read (pop)
    int iput;               // Next index to write (push)
    int count;              // Number of pending descriptors
    int fd[MAX_QUEUE];      // Array of connected descriptors
    pthread_mutex_t mutex;  // Mutex protecting queue state
    pthread_cond_t cond;    // Condition variable signaling available job
} Queue;

static Queue queue;
static pthread_t tids[NTHREADS];
static long thread_count[NTHREADS];

// Queue Initialization
void queue_init(Queue *q) {
    q->iget = 0;
    q->iput = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void queue_put(Queue *q, int fd) {
    pthread_mutex_lock(&q->mutex);

    // If queue is full, drop connection or block (simplified handling here)
    if (q->count >= MAX_QUEUE) {
        fprintf(stderr, "[MAIN] Queue full! Dropping connection fd=%d\n", fd);
        close(fd);
        pthread_mutex_unlock(&q->mutex);
        return;
    }

    q->fd[q->iput] = fd;
    if (++q->iput == MAX_QUEUE) q->iput = 0; // Wrap around
    q->count++;

    // Signal one waiting worker thread that a job is ready
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

int queue_get(Queue *q) {
    int fd;
    pthread_mutex_lock(&q->mutex);

    // Wait on condition variable while queue is empty
    while (q->count == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    fd = q->fd[q->iget];
    if (++q->iget == MAX_QUEUE) q->iget = 0; // Wrap around
    q->count--;

    pthread_mutex_unlock(&q->mutex);
    return fd;
}

// Worker Thread Main Function
void *worker_main(void *arg) {
    int thread_idx = *(int *)arg;
    free(arg);
    int connfd;
    char buf[BUFSIZE];
    char body[128];

    printf(
        "Worker Thread %d (TID %lu) initialized and waiting...\n", 
        thread_idx, (unsigned long)pthread_self()
    );

    for (;;) {
        // Block until a descriptor is available in the queue
        connfd = queue_get(&queue);

        // Track stats
        thread_count[thread_idx]++;

        // Service the client request
        int body_len = snprintf(body, sizeof(body), "Handled by Thread %d\n", thread_idx);
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
    return NULL;
}

// Signal Handler for Clean Shutdown & Stats
void sig_int(int signo) {
    printf("\n--- Prethreaded Server Workload Statistics ---\n");
    for (int i = 0; i < NTHREADS; i++) {
        printf(
            "Thread %d (TID %lu) handled %ld requests\n", 
            i, (unsigned long)tids[i], thread_count[i]
        );
    }
    printf("Server shutting down cleanly.\n");
    exit(0);
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;

    queue_init(&queue);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) { perror("socket error"); exit(1); }

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

    signal(SIGINT, sig_int);

    // Prethread Worker Pool
    for (int i = 0; i < NTHREADS; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        if (pthread_create(&tids[i], NULL, worker_main, arg) != 0) {
            perror("pthread_create error");
            exit(1);
        }
    }

    printf("[MAIN THREAD] Listening on port %d with %d worker threads...\n", PORT, NTHREADS);

    // Main Thread Accept Loop (Producer)
    for (;;) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if (connfd < 0) {
            perror("accept error");
            continue;
        }

        queue_put(&queue, connfd);
    }

    return 0;
}

// for i in {1..5000}; do nc -z localhost 8080 > /dev/null 2>&1; done

/*
Worker Thread 0 (TID 135956518131392) initialized and waiting...
Worker Thread 2 (TID 135956384446144) initialized and waiting...
Worker Thread 3 (TID 135956501345984) initialized and waiting...
Worker Thread 1 (TID 135956509738688) initialized and waiting...
Worker Thread 4 (TID 135956492953280) initialized and waiting...
[MAIN THREAD] Listening on port 8080 with 5 worker threads...
^C
--- Prethreaded Server Workload Statistics ---
Thread 0 (TID 135956518131392) handled 1002 requests
Thread 1 (TID 135956509738688) handled 1000 requests
Thread 2 (TID 135956384446144) handled 999 requests
Thread 3 (TID 135956501345984) handled 998 requests
Thread 4 (TID 135956492953280) handled 1001 requests
Server shutting down cleanly.
*/