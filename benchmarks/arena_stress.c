#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    uint64_t acc = 0;
    uint8_t* ptrs[256];
    size_t sizes[256];

    // 50,000 request waves * 256 variable-sized objects = 12,800,000 allocations
    // With equivalent CWE-14 memory scrubbing before free()
    for (uint32_t wave = 0; wave < 50000; wave++) {
        for (uint32_t obj = 0; obj < 256; obj++) {
            size_t sz = (size_t)(64 + ((wave + obj) % 192));
            uint8_t* p = (uint8_t*)malloc(sz);
            if (!p) abort();
            asm volatile("" : : "r"(p) : "memory");
            p[0] = (uint8_t)((wave + obj) % 251);
            p[sz - 1] = (uint8_t)((wave ^ obj) % 251);
            ptrs[obj] = p;
            sizes[obj] = sz;
            acc += (uint64_t)p[0] + (uint64_t)p[sz - 1];
        }
        for (uint32_t obj = 0; obj < 256; obj++) {
            memset(ptrs[obj], 0, sizes[obj]);
            asm volatile("" : : "r"(ptrs[obj]) : "memory");
            free(ptrs[obj]);
        }
    }

    printf("%llu\n", (unsigned long long)acc);
    return 0;
}
