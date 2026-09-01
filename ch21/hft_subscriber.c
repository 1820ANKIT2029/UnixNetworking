// gcc hft_subscriber.c -o subscriber-test

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
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(1);
    }

    // Allow multiple trading processes to bind to the same multicast port
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Bind to the multicast port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTICAST_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }

    // Join the Multicast Group via IP_ADD_MEMBERSHIP
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt IP_ADD_MEMBERSHIP failed");
        close(sockfd);
        exit(1);
    }

    printf(
        "[HFT TRADER] Joined multicast group %s:%d. Processing tick feeds...\n\n",
        MULTICAST_IP, MULTICAST_PORT
    );

    // Fast Market Data Parsing Loop
    while (1) {
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        uint64_t recv_timestamp_ns = get_time_ns();

        if (bytes_received < (ssize_t)sizeof(MarketHeader)) continue;

        MarketHeader *header = (MarketHeader *)buffer;

        // Verify message type 'P' (Price Update)
        if (header->msg_type == 'P' && bytes_received >= sizeof(PriceUpdateMsg)) {
            PriceUpdateMsg *msg = (PriceUpdateMsg *)buffer;

            // Calculate network transit latency in nanoseconds / microseconds
            uint64_t latency_ns = recv_timestamp_ns - msg->header.timestamp_ns;

            printf("[HFT ENGINE] Seq #%lu | Symbol: %s | Bid: $%.2f | Ask: $%.2f | Latency: %.3f µs\n",
                   msg->header.sequence_num,
                   msg->symbol,
                   msg->bid_price / 100.0,
                   msg->ask_price / 100.0,
                   latency_ns / 1000.0);
        }
    }

    // Leave group and close
    setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    close(sockfd);
    return 0;
}