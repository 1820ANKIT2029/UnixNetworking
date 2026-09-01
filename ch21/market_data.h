#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <stdint.h>

#define MULTICAST_IP   "239.255.10.20"
#define MULTICAST_PORT 9999

// Fixed-size binary message header
typedef struct __attribute__((packed)) {
    uint16_t msg_len;            // Size of total message
    uint8_t  msg_type;           // 'P' = Price Update, 'T' = Trade
    uint64_t sequence_num;       // Monotonically increasing sequence number
    uint64_t timestamp_ns;       // Nanosecond timestamp from exchange clock
} MarketHeader;

// Price Update Payload
typedef struct __attribute__((packed)) {
    MarketHeader header;
    char     symbol[8];   // e.g., "AAPL\0\0\0\0"
    uint32_t bid_price;   // Stored as integer (e.g., $150.25 -> 15025)
    uint32_t ask_price;   // Stored as integer
    uint32_t bid_size;    // Quantity available
    uint32_t ask_size;
} PriceUpdateMsg;

#endif