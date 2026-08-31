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

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Replace "eth0" with "lo", "eth0", "wlan0", or your active interface name
    const char *iface = "eth0";
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    printf("=== Interface Info for '%s' ===\n", iface);

    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        perror("ioctl");
        close(sock);
        return 1;
    }

    // Get Interface Index (SIOCGIFINDEX)
    if (ioctl(sock, SIOCGIFINDEX, &ifr) != -1) {
        printf("Interface Index (ifr_ifindex) : %d\n", ifr.ifr_ifindex);
    }

    struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    printf("IP address of %s: %s\n", ifr.ifr_name, inet_ntoa(ipaddr->sin_addr));

    // Get Netmask (SIOCGIFNETMASK)
    if (ioctl(sock, SIOCGIFNETMASK, &ifr) != -1) {
        struct sockaddr_in *mask = (struct sockaddr_in *)&ifr.ifr_netmask;
        printf("Netmask (ifr_netmask)         : %s\n", inet_ntoa(mask->sin_addr));
    }

    // Get Broadcast Address (SIOCGIFBRDADDR)
    if (ioctl(sock, SIOCGIFBRDADDR, &ifr) != -1) {
        struct sockaddr_in *brd = (struct sockaddr_in *)&ifr.ifr_broadaddr;
        printf("Broadcast Addr (ifr_broadaddr): %s\n", inet_ntoa(brd->sin_addr));
    }

    // Get Hardware / MAC Address (SIOCGIFHWADDR)
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) != -1) {
        unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
        printf("Hardware/MAC (ifr_hwaddr)     : %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    // Get Flags (SIOCGIFFLAGS)
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) != -1) {
        printf("Flags (ifr_flags)             : 0x%04X (", ifr.ifr_flags);
        if (ifr.ifr_flags & IFF_UP)          printf("UP ");
        if (ifr.ifr_flags & IFF_BROADCAST)   printf("BROADCAST ");
        if (ifr.ifr_flags & IFF_LOOPBACK)    printf("LOOPBACK ");
        if (ifr.ifr_flags & IFF_RUNNING)     printf("RUNNING ");
        if (ifr.ifr_flags & IFF_MULTICAST)   printf("MULTICAST ");
        printf(")\n");
    }

    // Get MTU (SIOCGIFMTU)
    if (ioctl(sock, SIOCGIFMTU, &ifr) != -1) {
        printf("MTU (ifr_mtu)                 : %d\n", ifr.ifr_mtu);
    }

    // Get Metric (SIOCGIFMETRIC)
    if (ioctl(sock, SIOCGIFMETRIC, &ifr) != -1) {
        printf("Metric (ifr_metric)           : %d\n", ifr.ifr_metric);
    }

    // Get TX Queue Length (SIOCGIFTXQLEN)
    if (ioctl(sock, SIOCGIFTXQLEN, &ifr) != -1) {
        printf("TX Queue Length (ifr_qlen)    : %d\n", ifr.ifr_qlen);
    }

    close(sock);
    return 0;
}
