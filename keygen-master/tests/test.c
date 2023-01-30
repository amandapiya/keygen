#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main() {
    srand(time(NULL));
    for (int i = 0; i < 35; i++) {
        uint16_t a = rand();
        uint16_t b = rand();
        uint16_t c = rand();
        uint16_t d = rand();
        uint64_t full_rand = 0;
        full_rand |= a; 
        full_rand |= ((uint64_t)b << 16);
        full_rand |= ((uint64_t)b << 32);
        full_rand |= ((uint64_t)b << 48);
        // printf("%d, %lx, %lu\n", i, full_rand, (full_rand >> 63) & 0x01);
        uint64_t k = 0xFFFFFFFFFFFFFFFF;

        printf("%d, %llx, %llu\n", i, k << i, (full_rand >> 63) & 0x01);
    }
}