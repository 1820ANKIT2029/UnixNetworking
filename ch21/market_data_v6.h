#ifndef MARKET_DATA_V6_H
#define MARKET_DATA_V6_H

#include <stdint.h>

#define MULTICAST_IPV6 "ff05::10:20"  // Site-Local IPv6 Multicast Address
#define MULTICAST_PORT 9999

typedef struct __attribute__((packed)) {
    uint16_t msg_len;
    uint8_t  msg_type;      // 'P' = Price Update
    uint64_t sequence_num;
    uint64_t timestamp_ns;
} MarketHeader;

typedef struct __attribute__((packed)) {
    MarketHeader header;
    char     symbol[8];
    uint32_t bid_price;
    uint32_t ask_price;
    uint32_t bid_size;
    uint32_t ask_size;
} PriceUpdateMsg;

#endif