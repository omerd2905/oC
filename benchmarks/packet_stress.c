#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    const uint8_t* ptr;
    size_t len;
} c_slice;

static uint64_t parse_wire_frame_unchecked(const uint8_t* frame, uint32_t seed) {
    const uint8_t* eth = frame;
    const uint8_t* ip = frame + 14;
    const uint8_t* tcp = frame + 34;

    uint32_t proto = (uint32_t)ip[9];
    uint32_t flags = (uint32_t)tcp[13];
    uint32_t mac_tail = (uint32_t)eth[5];

    uint64_t hash = 2166136261ULL;
    hash = (hash ^ (uint64_t)proto) * 16777619ULL;
    hash = (hash ^ (uint64_t)flags) * 16777619ULL;
    hash = (hash ^ (uint64_t)mac_tail) * 16777619ULL;
    hash = (hash ^ (uint64_t)seed) * 16777619ULL;
    return hash & 0xffffffffULL;
}

int main(void) {
    uint8_t raw_buf[64] = {0};
    for (size_t k = 0; k < 64; k++) {
        raw_buf[k] = (uint8_t)((k * 17) + 3);
    }

    uint64_t acc = 0;
    for (uint32_t i = 0; i < 25000000; i++) {
        raw_buf[5] = (uint8_t)((i + (uint32_t)acc) & 255);
        raw_buf[23] = (uint8_t)((i >> 3) & 255);
        raw_buf[47] = (uint8_t)((i ^ (uint32_t)acc) & 255);
        asm volatile("" : "+m"(raw_buf));
        acc += parse_wire_frame_unchecked(raw_buf, i);
    }

    printf("%llu\n", (unsigned long long)acc);
    return 0;
}
