#include <stdio.h>
#include <stdint.h>

uint64_t fib(uint64_t n) {
    if (n <= 1) return n;
    return fib(n - 1) + fib(n - 2);
}

uint64_t collatz_steps(uint64_t start) {
    uint64_t n = start;
    uint64_t steps = 0;
    while (n > 1) {
        if ((n % 2) == 0) {
            n = n / 2;
        } else {
            n = (n * 3) + 1;
        }
        steps++;
    }
    return steps;
}

int main(void) {
    uint64_t f38 = fib(38);
    uint64_t total_steps = 0;
    for (uint64_t seed = 1; seed <= 200000; seed++) {
        total_steps += collatz_steps(seed);
    }
    printf("%llu\n", (unsigned long long)(f38 + total_steps));
    return 0;
}
