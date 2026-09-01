// gcc hft_subscriber_v6.c -o subscriber_v6-test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "market_data_v6.h"

uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    int sockfd;
    struct sockaddr_in6 addr;
    struct ipv6_mreq mreq6;
    char buffer[1024];

    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket AF_INET6 failed");
        exit(1);
    }

    // Allow port reuse for multiple subscriber processes
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Bind to the IPv6 multicast port
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(MULTICAST_PORT);
    addr.sin6_addr = in6addr_any; // Bind to all IPv6 interfaces

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind IPv6 failed");
        close(sockfd);
        exit(1);
    }

    // Join IPv6 Multicast Group via IPV6_JOIN_GROUP
    memset(&mreq6, 0, sizeof(mreq6));
    inet_pton(AF_INET6, MULTICAST_IPV6, &mreq6.ipv6mr_multiaddr);
    mreq6.ipv6mr_interface = 0; // 0 = system chooses default interface

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) < 0) {
        perror("setsockopt IPV6_JOIN_GROUP failed");
        close(sockfd);
        exit(1);
    }

    printf("[HFT ENGINE IPv6] Joined group [%s]:%d. Waiting for market feed...\n\n",
           MULTICAST_IPV6, MULTICAST_PORT);

    // High-Speed Processing Loop
    while (1) {
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        uint64_t recv_timestamp_ns = get_time_ns();

        if (bytes_received < (ssize_t)sizeof(MarketHeader)) continue;

        MarketHeader *header = (MarketHeader *)buffer;

        if (header->msg_type == 'P' && bytes_received >= sizeof(PriceUpdateMsg)) {
            PriceUpdateMsg *msg = (PriceUpdateMsg *)buffer;

            uint64_t latency_ns = recv_timestamp_ns - msg->header.timestamp_ns;

            printf("[HFT ENGINE IPv6] Seq #%lu | Symbol: %s | Bid: $%.2f | Ask: $%.2f | Latency: %.3f µs\n",
                   msg->header.sequence_num,
                   msg->symbol,
                   msg->bid_price / 100.0,
                   msg->ask_price / 100.0,
                   latency_ns / 1000.0);
        }
    }

    // Leave IPv6 Multicast group
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6));
    close(sockfd);
    return 0;
}