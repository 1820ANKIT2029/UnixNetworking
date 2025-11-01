#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

#define MAXLINE 1024
#define SCTP_MAXLINE 1024
#define SERV_MAX_SCTP_STRM 100

void doit(int sock_fd, struct sockaddr *to, socklen_t tolen);
void doit_all(int sock_fd, struct sockaddr *to, socklen_t tolen);

int SERV_PORT = 8002;

int main(int argc, char **argv) {
    int client_fd;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_len;
    struct sctp_event_subscribe evnts;
    int echo_to_all = 0;

    if(argc < 2) {
        printf("usage: %s <IP-Addr> [echo]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(argc > 2) {
        printf("Echoing messages to all streams\n");
        echo_to_all = 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    client_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if(client_fd == -1) {
        printf("error in socket init\n");
        exit(1);
    }

    memset(&evnts, 0, sizeof(evnts));
    evnts.sctp_data_io_event = 1;
    setsockopt(client_fd, SOL_SOCKET, SCTP_EVENTS, &evnts, sizeof(evnts));

    if(echo_to_all == 0) {
        doit(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    }
    else {
        doit_all(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    }
 
    close(client_fd);
}

void doit(int sock_fd, struct sockaddr *to, socklen_t tolen) {
    struct sockaddr_in peeraddr;
    struct sctp_sndrcvinfo sri;
    char sendline[MAXLINE], recvline[MAXLINE];
    socklen_t len;
    int out_sz, rd_sz;
    int msg_flags;

    memset(&sri, 0, sizeof(sri));
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if(sendline[0] != '[') {
            printf ("Error, line must be of the form '[streamnum] text '\n");
            continue;
        }
        sri.sinfo_stream = strtol (&sendline [1], NULL, 0);
        out_sz = strlen (sendline);
        sctp_sendmsg (
            sock_fd, sendline, out_sz,
            to, tolen, 0, 0, sri.sinfo_stream, 0, 0
        );
        len = sizeof (peeraddr);
        rd_sz = sctp_recvmsg (
            sock_fd, recvline, sizeof (recvline),
            (struct sockaddr *) &peeraddr, &len, &sri, &msg_flags
        );
        printf (
            "From str:%d seq:%d (assoc:0x%x):",
            sri.sinfo_stream, sri.sinfo_ssn, (int) sri.sinfo_assoc_id
        );
        printf ("%.*s", rd_sz, recvline);
    }
}

void doit_all(int sock_fd, struct sockaddr *to, socklen_t tolen) {
    struct sockaddr_in peeraddr;
    struct sctp_sndrcvinfo sri;
    char sendline[SCTP_MAXLINE], recvline[SCTP_MAXLINE];
    socklen_t len;
    int rd_sz, i, strsz;
    int msg_flags;

    memset(sendline, 0, sizeof(sendline));
    memset(&sri, 0, sizeof(sri));
    while(fgets(sendline, SCTP_MAXLINE - 9, stdin) != NULL) {
        strsz = strlen(sendline);
        if (sendline[strsz - 1] == '\n') {
            sendline[strsz - 1] = '\0';
            strsz--;
        }

        for (i = 0; i < SERV_MAX_SCTP_STRM; i++) {
            snprintf(
                sendline + strsz, sizeof (sendline) - strsz,
                ".msg. %d", i
            ) ;
            sctp_sendmsg (
                sock_fd, sendline, sizeof(sendline),
                to, tolen, 0, 0, i, 0, 0
            );
        }
        for (i = 0; i < SERV_MAX_SCTP_STRM; i++) {
            len = sizeof(peeraddr);
            rd_sz = sctp_recvmsg(
                sock_fd, recvline, sizeof (recvline),
                (struct sockaddr *) &peeraddr, &len, &sri, &msg_flags
            );
            printf(
                "From str:%d seq:%d (assoc: 0X%X) :",
                sri.sinfo_stream, sri.sinfo_ssn,
                (int) sri.sinfo_assoc_id
            );
            printf ("%.*s\n", rd_sz, recvline);
        }
    }
}