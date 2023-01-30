#include "rsa.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "base64.h"

#define RSA_KEY_STRING = "ssh-rsa"

void rsa_init(rsa_context *ctx) {
    ctx->ver = 0;
    big_init(&ctx->N);
    big_init(&ctx->E);
    big_init(&ctx->D);
    big_init(&ctx->P);
    big_init(&ctx->Q);
    big_init(&ctx->DP);
    big_init(&ctx->DQ);
    big_init(&ctx->QP);

    big_init(&ctx->RN);
    big_init(&ctx->RP);
    big_init(&ctx->RQ);
    srand(time(NULL));
}

void rsa_free(rsa_context *ctx) {
    big_free(&ctx->N);
    big_free(&ctx->E);

    big_free(&ctx->D);
    big_free(&ctx->P);
    big_free(&ctx->Q);

    big_free(&ctx->DP);
    big_free(&ctx->DQ);
    big_free(&ctx->QP);

    big_free(&ctx->RN);
    big_free(&ctx->RP);
    big_free(&ctx->RQ);
    free(ctx);
}

int rsa_gen_key(rsa_context *ctx, size_t nbits, big_uint exponent) {
    big_set_nonzero(&ctx->E, exponent);

    bigint ONE;
    big_set_nonzero(&ONE, 1);
    
    bool complete = false;
    while (!complete) {
        
        big_gen_prime(&ctx->P, nbits / 2);
        big_gen_prime(&ctx->Q, nbits / 2);
        big_mul(&ctx->N, &ctx->P, &ctx->Q);

        bigint abs_p_minus_q;
        big_init(&abs_p_minus_q);
        big_sub(&abs_p_minus_q, &ctx->P, &ctx->Q);
        if (abs_p_minus_q.signum < 0) {
            abs_p_minus_q.signum = 1;
        }

        bigint pwr2;
        big_init(&pwr2);
        big_two_to_pwr(&pwr2, nbits / 2 - 100);

        if (big_cmp(&abs_p_minus_q, &pwr2) < 1) {
            big_free(&abs_p_minus_q);
            continue;
        }

        big_free(&abs_p_minus_q);

        bigint p_minus_1, q_minus_1;
        big_init(&p_minus_1);
        big_init(&q_minus_1);

        big_sub(&p_minus_1, &ctx->P, &ONE);
        big_sub(&q_minus_1, &ctx->Q, &ONE);

        bigint totient;
        big_init(&totient);
        big_mul(&totient, &p_minus_1, &q_minus_1); 

        bigint gcd;
        big_init(&gcd);
        big_gcd(&gcd, &ctx->E, &totient);

        big_free(&totient);

        if (big_cmp(&gcd, &ONE) != 0) {
            big_free(&gcd);
            continue; 
        }

        // LCM(p-1, q-1) = (p-1)(q-1) / (gcd(p-1, q-1))
        big_gcd(&gcd, &p_minus_1, &q_minus_1);
        bigint lcm;
        big_init(&lcm);
        big_mul(&lcm, &p_minus_1, &q_minus_1);
        big_div(&lcm, NULL, &lcm, &gcd);

        big_inv_mod(&ctx->D, &ctx->E, &lcm);

        big_two_to_pwr(&pwr2, nbits / 2);
        if (big_cmp(&ctx->D, &pwr2) != 1) {
            continue;
        }

        big_mod(&ctx->DP, &ctx->D, &p_minus_1);
        big_mod(&ctx->DQ, &ctx->D, &q_minus_1);
        big_inv_mod(&ctx->QP, &ctx->Q, &ctx->P);
    }
    return 0;
}

int rsa_write_public_key(const rsa_context *ctx, FILE *file) {
    // (length = 7, 'ssh-rsa', len(e), e, len(n), n)
    
    // allocate uint8_t * buffer to hold all this
    // encode it in base 64 -> string
    // concat it with "ssh-rsa " in human readable format
    // write this all to `file`
    size_t size = 3 * sizeof(uint64_t);
    size_t e_len_bits = big_bitlen(&ctx->E);
    size_t n_len_bits = big_bitlen(&ctx->N);

    size_t e_len_bytes = e_len_bits / 8 + (e_len_bits % 8 == 0 ? 0 : 1);
    size_t n_len_bytes = n_len_bits / 8 + (n_len_bits % 8 == 0 ? 0 : 1);

    size += e_len_bytes + n_len_bytes;
    size += 7; // ssh-rsa

    uint8_t *buf = calloc(size, sizeof(uint8_t));

    // set first 8 bytes to 0x07
    uint64_t *first_buf = (uint64_t *)buf; 
    *first_buf = 7;

    // set next 7 bytes to 'ssh-rsa' in ASCII
    char *str = (char *)(&buf[8]);
    strcpy(str, "ssh-rsa");

    uint64_t *second_len = (uint64_t *)(&buf[7 + 8]); 
    *second_len = e_len_bytes;

    uint8_t *e_buf = &buf[7 + 8 + 8];
    big_write_binary(&ctx->E, buf, e_len_bytes);

    uint64_t *third_len = (uint64_t *)(&buf[7 + 8 + 8 + e_len_bytes]); 
    *third_len = n_len_bytes;

    uint8_t *n_buf = (uint8_t *)(&buf[7 + 8 + 8 + e_len_bytes + 8]); 
    big_write_binary(&ctx->N, buf, n_len_bytes);

    size_t olen;
    char base64out[256];
    base64_encode(base64out, 256, &olen, buf, size);

    fwrite("ssh-rsa ", strlen("ssh-rsa") - 1, 1, file);
    fwrite(base64out, olen, 1, file);

    return 0;
}
