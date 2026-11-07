#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) {
    uint8_t version;
    uint16_t port;
    uint32_t seq;
    uint32_t checksum;
} Packet;

static inline uint32_t process_packet(const Packet* pkt) {
    uint32_t sum = (uint32_t)pkt->version + (uint32_t)pkt->port + pkt->seq + pkt->checksum;
    return sum;
}

int main(void) {
    printf("Starting 10,000,000 packet benchmark...\n");
    Packet pkt = {
        .version = 1,
        .port = 8080,
        .seq = 100,
        .checksum = 42,
    };

    uint64_t total = 0;
    uint32_t i = 0;
    while (i < 10000000) {
        pkt.seq = i;
        pkt.checksum = pkt.checksum + 1;
        uint32_t p = process_packet(&pkt);
        total = total + (uint64_t)p;
        i = i + 1;
    }

    printf("Completed 10,000,000 iterations.\n");
    printf("Total checksum:\n");
    printf("%llu\n", (unsigned long long)total);
    return 0;
}
