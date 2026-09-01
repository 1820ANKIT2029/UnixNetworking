// gcc exchange_publisher.c -o publisher-test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "market_data.h"

uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    int sockfd;
    struct sockaddr_in mcast_addr;
    uint64_t seq_num = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(1);
    }

    // Set Multicast TTL (1 = restrict to local network segment)
    unsigned char ttl = 1;
    setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    // Configure Multicast Target
    memset(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_port = htons(MULTICAST_PORT);
    inet_pton(AF_INET, MULTICAST_IP, &mcast_addr.sin_addr);

    printf("[EXCHANGE] Started broadcasting market data to %s:%d...\n", MULTICAST_IP, MULTICAST_PORT);

    PriceUpdateMsg msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.symbol, "AAPL", 8);

    uint32_t base_price = 18000; // $180.00

    // Simulate publishing market tick updates every 500ms
    while (1) {
        msg.header.msg_len = sizeof(PriceUpdateMsg);
        msg.header.msg_type = 'P';
        msg.header.sequence_num = seq_num++;
        msg.header.timestamp_ns = get_time_ns();

        // Simulate small price fluctuations
        msg.bid_price = base_price + (rand() % 10);
        msg.ask_price = msg.bid_price + 5; // $0.05 spread
        msg.bid_size = 100 * ((rand() % 10) + 1);
        msg.ask_size = 100 * ((rand() % 10) + 1);

        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&mcast_addr, sizeof(mcast_addr));

        printf("[EXCHANGE] Broadcasted Seq #%lu | %s | Bid: $%.2f (%u) | Ask: $%.2f (%u)\n",
               msg.header.sequence_num, msg.symbol,
               msg.bid_price / 100.0, msg.bid_size,
               msg.ask_price / 100.0, msg.ask_size);

        usleep(500000); // 500ms delay
    }

    close(sockfd);
    return 0;
}