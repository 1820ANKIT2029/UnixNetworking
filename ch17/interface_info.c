#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock;
    struct ifreq ifr;
    struct ifi_info 

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        perror("ioctl");
        close(sock);
        return 1;
    }

    struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    printf("IP address of %s: %s\n", ifr.ifr_name, inet_ntoa(ipaddr->sin_addr));

    close(sock);
    return 0;
}
