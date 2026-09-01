// gcc exchange_publisher_v6.c -o publisher_v6-test

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
    struct sockaddr_in6 mcast_addr;
    uint64_t seq_num = 1;

    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket AF_INET6 failed");
        exit(1);
    }

    // Set Multicast Hop Limit (IPv6 equivalent of IPv4 TTL)
    int hops = 1;
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops)) < 0) {
        perror("setsockopt IPV6_MULTICAST_HOPS failed");
        close(sockfd);
        exit(1);
    }

    // Configure IPv6 Multicast Destination
    memset(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin6_family = AF_INET6;
    mcast_addr.sin6_port = htons(MULTICAST_PORT);
    inet_pton(AF_INET6, MULTICAST_IPV6, &mcast_addr.sin6_addr);

    printf("[EXCHANGE IPv6] Publishing market feed to [%s]:%d...\n", MULTICAST_IPV6, MULTICAST_PORT);

    PriceUpdateMsg msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.symbol, "NVDA", 8);

    uint32_t base_price = 12000; // $120.00

    while (1) {
        msg.header.msg_len = sizeof(PriceUpdateMsg);
        msg.header.msg_type = 'P';
        msg.header.sequence_num = seq_num++;
        msg.header.timestamp_ns = get_time_ns();

        msg.bid_price = base_price + (rand() % 15);
        msg.ask_price = msg.bid_price + 2; // $0.02 spread
        msg.bid_size = 100 * ((rand() % 5) + 1);
        msg.ask_size = 100 * ((rand() % 5) + 1);

        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&mcast_addr, sizeof(mcast_addr));

        printf("[EXCHANGE IPv6] Sent Seq #%lu | %s | Bid: $%.2f (%u) | Ask: $%.2f (%u)\n",
               msg.header.sequence_num, msg.symbol,
               msg.bid_price / 100.0, msg.bid_size,
               msg.ask_price / 100.0, msg.ask_size);

        usleep(500000); // 500ms
    }

    close(sockfd);
    return 0;
}