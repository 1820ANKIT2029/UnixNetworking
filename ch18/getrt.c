#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>

#define BUFSIZE 8192

int main(int argc, char **argv) {
    int fd;
    struct {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[1024];
    } req;

    char res_buf[BUFSIZE];
    ssize_t status;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <destination-ip>\n", argv[0]);
        return 1;
    }

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        perror("socket AF_NETLINK");
        return 1;
    }

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_GETROUTE;
    req.r.rtm_family = AF_INET;

    // Attach target IP address attribute (RTA_DST)
    struct rtattr *rta = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_type = RTA_DST;
    rta->rta_len = RTA_LENGTH(sizeof(struct in_addr));
    inet_pton(AF_INET, argv[1], RTA_DATA(rta));

    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_LENGTH(sizeof(struct in_addr));

    if (send(fd, &req, req.n.nlmsg_len, 0) < 0) {
        perror("send");
        close(fd);
        return 1;
    }

    status = recv(fd, res_buf, sizeof(res_buf), 0);
    if (status < 0) {
        perror("recv");
        close(fd);
        return 1;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)res_buf;
    if (NLMSG_OK(nlh, status) && nlh->nlmsg_type == RTM_NEWROUTE) {
        struct rtmsg *rt = (struct rtmsg *)NLMSG_DATA(nlh);
        struct rtattr *attr = (struct rtattr *)RTM_RTA(rt);
        int len = RTM_PAYLOAD(nlh);

        printf("Route lookup for %s:\n", argv[1]);

        for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
            if (attr->rta_type == RTA_GATEWAY) {
                struct in_addr *gate = (struct in_addr *)RTA_DATA(attr);
                printf("  Gateway : %s\n", inet_ntoa(*gate));
            }
            if (attr->rta_type == RTA_OIF) {
                int if_index = *(int *)RTA_DATA(attr);
                char if_name[IF_NAMESIZE];
                if_indextoname(if_index, if_name);
                printf("  Interface: %s (Index: %d)\n", if_name, if_index);
            }
        }
    } 
    else {
        printf("No route found or invalid response received.\n");
    }

    close(fd);
    return 0;
}