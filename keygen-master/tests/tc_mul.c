#include <sys/time.h>
#include <stdio.h>
#include <assert.h>
#include "bigint.h"

#include <sys/time.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // two arguments, the multiplier and the multiplicand
    assert(argc == 3);

    bigint A;
    big_init(&A);
    big_read_string(&A, argv[1]);
    bigint B;
    big_init(&B);
    big_read_string(&B, argv[2]);

    struct timeval start, stop;

    gettimeofday(&start, NULL);
    big_mul_tc(&A, &A, &B);
    gettimeofday(&stop, NULL);

    printf("%lu\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec); 

    big_free(&A);
    big_free(&B);
}