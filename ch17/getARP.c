#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

int main(int argc, char **argv) {
    int sockfd;
    struct ifaddrs *ifap, *ifa;
    struct arpreq arpreq;
    struct sockaddr_in *sin;
    unsigned char *ptr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        exit(1);
    }

    // Get list of all interfaces
    if (getifaddrs(&ifap) < 0) {
        perror("getifaddrs error");
        close(sockfd);
        exit(1);
    }

    // Loop through interfaces
    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        // Only process IPv4 addresses
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        sin = (struct sockaddr_in *)ifa->ifa_addr;
        printf("Interface: %-6s | IP: %s\n", ifa->ifa_name, inet_ntoa(sin->sin_addr));

        // Prepare ARP query structure
        memset(&arpreq, 0, sizeof(arpreq));
        memcpy(&arpreq.arp_pa, ifa->ifa_addr, sizeof(struct sockaddr_in));
        strncpy(arpreq.arp_dev, ifa->ifa_name, sizeof(arpreq.arp_dev) - 1);

        // Issue SIOCGARP query
        if (ioctl(sockfd, SIOCGARP, &arpreq) < 0) {
            perror("  ioctl SIOCGARP");
            continue;
        }

        // Print MAC address if found in ARP table
        ptr = (unsigned char *)arpreq.arp_ha.sa_data;
        printf("  MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
    }

    freeifaddrs(ifap);
    close(sockfd);
    return 0;
}