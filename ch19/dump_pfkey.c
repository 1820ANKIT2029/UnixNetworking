#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/pfkeyv2.h>
#include <errno.h>

#define BUFSIZE 8192

const char *get_msg_type_str(uint8_t type) {
    switch (type) {
        case SADB_RESERVED:   return "SADB_RESERVED";
        case SADB_GETSPI:     return "SADB_GETSPI";
        case SADB_UPDATE:     return "SADB_UPDATE";
        case SADB_ADD:        return "SADB_ADD";
        case SADB_DELETE:     return "SADB_DELETE";
        case SADB_GET:        return "SADB_GET";
        case SADB_ACQUIRE:    return "SADB_ACQUIRE";
        case SADB_REGISTER:   return "SADB_REGISTER";
        case SADB_EXPIRE:     return "SADB_EXPIRE";
        case SADB_FLUSH:      return "SADB_FLUSH";
        case SADB_DUMP:       return "SADB_DUMP";
        case SADB_X_PROMISC:  return "SADB_X_PROMISC";
        default:              return "UNKNOWN_TYPE";
    }
}

const char *get_satype_str(uint8_t satype) {
    switch (satype) {
        case SADB_SATYPE_UNSPEC: return "UNSPEC";
        case SADB_SATYPE_AH:     return "AH";
        case SADB_SATYPE_ESP:    return "ESP";
        case SADB_SATYPE_RSVP:   return "RSVP";
        case SADB_SATYPE_OSPFV2: return "OSPFV2";
        case SADB_SATYPE_RIPV2:  return "RIPV2";
        case SADB_SATYPE_MIP:    return "MIP";
        default:                 return "UNKNOWN_SATYPE";
    }
}

int register_socket(int sockfd, uint8_t satype) {
    struct sadb_msg msg;

    memset(&msg, 0, sizeof(msg));
    msg.sadb_msg_version = PF_KEY_V2;
    msg.sadb_msg_type = SADB_REGISTER;
    msg.sadb_msg_satype = satype;
    msg.sadb_msg_len = sizeof(msg) / 8;
    msg.sadb_msg_pid = getpid();
    msg.sadb_msg_seq = 1;

    if (write(sockfd, &msg, sizeof(msg)) < 0) {
        perror("write SADB_REGISTER failed");
        return -1;
    }
    return 0;
}

int main() {
    int sockfd;
    ssize_t n;
    char buf[BUFSIZE];
    struct sadb_msg *msg;

    sockfd = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
    if (sockfd < 0) {
        perror("socket(PF_KEY) failed (Requires root / CAP_NET_ADMIN privileges)");
        exit(EXIT_FAILURE);
    }

    printf("PF_KEY socket opened successfully (fd=%d).\n", sockfd);

    printf("Registering socket for ESP and AH events...\n");
    register_socket(sockfd, SADB_SATYPE_ESP);
    register_socket(sockfd, SADB_SATYPE_AH);

    printf("Listening for PF_KEY messages from kernel... (Press Ctrl+C to exit)\n\n");

    for (;;) {
        n = read(sockfd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("read error");
            break;
        }

        if (n < (ssize_t)sizeof(struct sadb_msg)) {
            fprintf(stderr, "Received truncated message (%zd bytes)\n", n);
            continue;
        }

        msg = (struct sadb_msg *)buf;

        printf("--------------------------------------------------\n");
        printf("Received PF_KEY Message:\n");
        printf("  Version   : %d\n", msg->sadb_msg_version);
        printf("  Type      : %s (%d)\n", get_msg_type_str(msg->sadb_msg_type), msg->sadb_msg_type);
        printf("  SA Type   : %s (%d)\n", get_satype_str(msg->sadb_msg_satype), msg->sadb_msg_satype);
        printf("  Len (bytes): %d (header len: %d x 8 bytes)\n", msg->sadb_msg_len * 8, msg->sadb_msg_len);
        printf("  Errno     : %d (%s)\n", msg->sadb_msg_errno, 
               msg->sadb_msg_errno ? strerror(msg->sadb_msg_errno) : "Success");
        printf("  Sequence  : %u\n", msg->sadb_msg_seq);
        printf("  PID       : %u\n", msg->sadb_msg_pid);
        printf("--------------------------------------------------\n\n");
    }

    close(sockfd);
    return 0;
}