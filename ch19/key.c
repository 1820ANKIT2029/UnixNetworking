#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/pfkeyv2.h>

#define BUFSIZE 4096

int main() {
    int fd;
    ssize_t n;
    char buf[BUFSIZE];
    struct sadb_msg msg;

    fd = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
    if (fd < 0) {
        perror("socket PF_KEY failed (Requires root / CAP_NET_ADMIN)");
        exit(1);
    }

    memset(&msg, 0, sizeof(msg));
    msg.sadb_msg_version = PF_KEY_V2;
    msg.sadb_msg_type = SADB_DUMP;
    msg.sadb_msg_satype = SADB_SATYPE_UNSPEC; // Dump all protocol SAs (AH & ESP)
    msg.sadb_msg_len = sizeof(msg) / 8;        // Length specified in 64-bit (8-byte) words
    msg.sadb_msg_pid = getpid();
    msg.sadb_msg_seq = 1;

    if (write(fd, &msg, sizeof(msg)) < 0) {
        perror("write to PF_KEY socket failed");
        close(fd);
        exit(1);
    }

    printf("Sent SADB_DUMP request. Reading kernel response...\n");

    n = read(fd, buf, sizeof(buf));
    if (n < 0) perror("read error");
    else {
        struct sadb_msg *hdr = (struct sadb_msg *)buf;
        printf("Received PF_KEY message type: %d | Length: %d bytes | Errno: %d\n",
               hdr->sadb_msg_type, hdr->sadb_msg_len * 8, hdr->sadb_msg_errno);
        
        if (hdr->sadb_msg_errno == 0)
            printf("SADB query successful.\n");
        else
            printf("SADB returned error code: %s\n", strerror(hdr->sadb_msg_errno));
    }

    close(fd);
    return 0;
}