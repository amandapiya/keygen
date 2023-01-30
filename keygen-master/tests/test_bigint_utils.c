#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "bigint.h"
#include "test_utils.h"

void test_big_init_free() {
    bigint *test = malloc(sizeof(bigint));
    big_init(test);
    assert(test->signum == 0);
    assert(test->num_limbs == 0);
    assert(test->data == NULL);

    big_free(test);
    assert(test->data == NULL);
}

void big_copy_tester(int idx, char **params) {
    (void)idx;
    /* params is a 1-array */
    size_t olen = 0;
    bigint X;
    big_init(&X);
    big_read_string(&X, params[0]);

    bigint Y;
    big_init(&Y);
    big_copy(&Y, &X);

    char out[100];
    big_write_string(&Y, out, 100, &olen);
    assert(strcmp(out, params[0]) == 0);

    big_free(&X);
    big_free(&Y);
}

void big_read_write_tester(int idx, char **params) {
    (void)idx;
    bigint test;
    size_t olen;
    big_init(&test);
    big_read_string(&test, params[0]);
    char out[256];
    big_write_string(&test, out, 256, &olen);
    // printf("CASE %zu: %s, %s\n", idx, out, params[2]);
    assert((int) test.num_limbs == atoi(params[1]));
    assert(strcmp(out, params[2]) == 0);
    big_free(&test);
}

void big_read_tester_invalid(int idx, char **params) {
    (void)idx;
    bigint test;
    big_init(&test);
    int result = big_read_string(&test, params[0]);
    // printf("CASE %zu: %s, %d, %d\n", idx, params[0], result, ERR_BIGINT_BAD_INPUT_DATA);
    assert(result == ERR_BIGINT_BAD_INPUT_DATA);
    big_free(&test);
}


void big_read_write_buff_too_small_tester(int idx, char **params) {
    (void)idx;
    bigint test;
    size_t *olen = malloc(sizeof(size_t));
    big_init(&test);
    big_read_string(&test, params[0]);
    char out[6];
    big_write_string(&test, out, 6, olen);
    // printf("CASE %zu: %s, %s\n", idx, out, params[2]);
    assert((int) test.num_limbs == atoi(params[1]));
    // printf("out: %s\n", out);
    // printf("olen: %zu\n", *olen);
    int expeted_olen = strlen(params[2]) + 1;
    // printf("strlen(params[2] + 1): %zu\n", expeted_olen);
    assert(*olen == strlen(params[2]) + 1);
    big_free(&test);
    free(olen);
    // printf("\n");
}


void big_read_binary_tester(int idx, char **params) {
    (void)idx;
    bigint test;
    size_t olen;
    uint8_t buf[256];
    big_init(&test);
    printf("CASE %zu\n", idx);
    big_read_binary(&test, buf, 256);
    char out[256];
    big_write_string(&test, out, 256, &olen);
    printf("CASE %zu: %zu, %s\n", idx, out[255], params[2]);
}


void big_read_write_binary_tester(int idx, char **params) {
    printf("CASE %zu:\n", idx);
    (void)idx;
    bigint test;
    uint8_t buf[256];
    big_init(&test);
    // printf("CASE %zu\n", idx);
    big_read_string(&test, params[0]);

    // uint8_t biggest_uint8 = 255;
    // printf("buf = ");
    // for (int j = 0; j < strlen(params[0]); j+=4) { 
    //     buf[j] = biggest_uint8 & ((uint8_t *) &test.data)[j];
    //     printf("%02x", buf[j]);
    // }
    // printf("\n");

    
    // big_read_binary(&test, buf, 256);
    // printf("%zu\n", &test.data[0]);
    size_t olen = 256;
    uint8_t out[256];
    char actual[256];
    char char_out[256];
    big_write_binary(&test, out, olen);

    // printf("CASE %zu: %zu, %s\n", idx, out[255], params[2]);

    int i = 0;
    int j = 0;

    while (i < olen - 1 && out[i] == 0){        // olen - 1 to account for the 0 case
        i++;
    }

    // printf("out[i]:");
    
    for (i; i < olen; i++) {
        // char temp[3];
        // sprintf(&temp[0], "%02x", out[i]);
        // sprintf(&actual[2*i], "%02x", out[i]);     // causes a write memory access error
        sprintf(&actual[j], "%02x", out[i]);  
        j += 2;
        // printf("%d", out[i]);     
    }
    // printf("\n");


    printf("actual: %s\n", actual);
    printf("params: %s\n", params[0]);
    // printf("params[2]: %s\n", params[2]);

    // long answer = strtoul(actual, NULL, 16);
    // long given = strtoul(params[2], NULL, 10);
    // assert(answer == given);

    // printf("answer: %d\n", answer);
    // printf("given: %d\n", given);
    // printf("actual: %s\n", actual);
    // printf("params[2]: %s\n", params[2]);

    big_free(&test);
}

void big_add_tester(int idx, char **params) {
    (void)idx;
    bigint num1, num2, result;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&result);
    big_add(&result, &num1, &num2);
    char out[2048];
    big_write_string(&result, out, 2048, &olen);
    // printf("case %d: %s + %s = %s, %s\n", idx, params[0], params[1], out, params[2]);
    if (strcmp(out, params[2]) != 0) {
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&result);
}

void big_add_same_memory_same_tester(int idx, char **params){
    (void)idx;
    bigint num1, num2;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_add(&num1, &num1, &num2);
    char out1[2048];
    big_write_string(&num1, out1, 2048, &olen);
    if (strcmp(out1, params[2]) != 0) {
        printf("FAILED case %d: %s + %s = %s, %s\n", idx, params[0], params[1], out1, params[2]);
        assert(strcmp(out1, params[2]) == 0);
    }

    big_read_string(&num1, params[0]);   
    big_add(&num2, &num1, &num2);
    char out2[2048];
    big_write_string(&num2, out2, 2048, &olen);
    if (strcmp(out2, params[2]) != 0) {
        printf("FAILED case %d: %s + %s = %s, %s\n", idx, params[0], params[1], out2, params[2]);
        assert(strcmp(out2, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
}

void big_add_all_memory_same_tester(int idx, char **params){
    (void)idx;
    bigint num1;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_add(&num1, &num1, &num1);
    char out[128];
    big_write_string(&num1, out, 128, &olen);
    if (strcmp(out, params[2]) != 0) {
        printf("FAILED case %d: %s + %s = %s, %s\n", idx, params[0], params[1], out, params[2]);
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
}

void big_sub_tester(int idx, char **params) {
    (void)idx;
    bigint num1, num2, result;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&result);
    int d = big_sub(&result, &num1, &num2);
    char out[1024];
    big_write_string(&result, out, 1024, &olen);
    if (strcmp(out, params[2]) != 0) {
        printf("case #%d sub(%s, %s) = %s, %s \n", idx, params[0], params[1], out, params[2]);
        printf("return #: %d\n", d);
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&result);
}

void big_mul_tester(int idx, char **params) {
    (void)idx;
    bigint num1, num2, result;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&result);
    big_mul(&result, &num1, &num2);
    char out[384];
    big_write_string(&result, out, 384, &olen);
    if (strcmp(out, params[2]) != 0) {
        printf("case #%d %s * %s = %s, %s\n", idx, params[0], params[1], out, params[2]);
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&result);
}

void big_tc_mul_tester(int idx, char **params) {
    (void)idx;
    bigint num1, num2, result;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&result);
    big_mul_tc(&result, &num1, &num2);
    char out[2048];
    big_write_string(&result, out, 2048, &olen);
    if (strcmp(out, params[2]) != 0) {
        printf("case #%d %s * %s = %s, %s\n", idx, params[0], params[1], out, params[2]);
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&result);
}

void big_kara_split_tester(int idx, char **params){
    // A, B, m, lowA, highA, lowB, highB
    printf("case #%d\n", idx);
    (void)idx;
    bigint A, B, lowA, highA, lowB, highB;
    big_init(&A);
    big_read_string(&A, params[0]);
    big_init(&B);
    big_read_string(&B, params[1]);
    size_t m = atoi(params[2]);
    big_init(&lowA);
    big_init(&highA);
    big_init(&lowB);
    big_init(&highB);
    size_t m2 = ceil((double) m / 2.0);         // from big_mul_karatsuba_helper
    karatsuba_low_high(&lowA, &highA, m2, &A);
    karatsuba_low_high(&lowB, &highB, m2, &B);
    size_t lowA_olen;
    char lowA_out[256];
    big_write_string(&lowA, lowA_out, 256, &lowA_olen);
    size_t highA_olen;
    char highA_out[256];
    big_write_string(&highA, highA_out, 256, &highA_olen);
    size_t lowB_olen;
    char lowB_out[256];
    big_write_string(&lowB, lowB_out, 256, &lowB_olen);
    size_t highB_olen;
    char highB_out[256];
    big_write_string(&highB, highB_out, 256, &highB_olen);
    if (strcmp(lowA_out, params[3]) != 0 || strcmp(highA_out, params[4]) != 0 
     || strcmp(lowB_out, params[5]) != 0 || strcmp(highB_out, params[6]) != 0) {
        printf("case #%d %s, %s\n", idx, params[0], params[1]);
        printf("m = %zu, m2 = %zu\n", m, m2);
        printf("Expected: LowA %s, HighA %s, LowB %s, HighB %s\n", params[3], params[4], params[5], params[6]);
        printf("Computed: LowA %s, HighA %s, LowB %s, HighB %s\n", lowA_out, highA_out, lowB_out, highB_out);
        assert(false);
    }
    big_free(&A);
    big_free(&B);
    big_free(&lowA);
    big_free(&highA);
    big_free(&lowB);
    big_free(&highB);
}

void big_karatsuba_mul_tester(int idx, char **params) {
    (void)idx;
    // printf("case #%d\n", idx);
    bigint num1, num2, result;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&result);
    int k = big_mul_karatsuba(&result, &num1, &num2);
    char out[1024];
    big_write_string(&result, out, 1024, &olen);
    if (strcmp(out, params[2]) != 0) {
        printf("case #%d %s * %s = %s, %s\n", idx, params[0], params[1], out, params[2]);
        printf("error value type: %d\n", k);
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&result);
}


void big_shr_tester(int idx, char **params) {
    (void)idx;
    bigint num1;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    
    size_t shift = strtoul(params[1], NULL, 10);
    big_shr(&num1, &num1, shift);
    char out[4096];
    big_write_string(&num1, out, 4096, &olen);
    printf("case #%d %s >> %s = %s, %s\n", idx, params[0], params[1], out, params[2]);
    if (strcmp(out, params[2]) != 0) {
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
}

void big_div_by_3_tester(int idx, char **params) {
    (void)idx;
    bigint num1;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_fast_divide_by_3(&num1, &num1);

    char out[4096];
    big_write_string(&num1, out, 4096, &olen);
    printf("case #%d %s // 3 = %s, %s\n", idx, params[0], out, params[1]);
    if (strcmp(out, params[1]) != 0) {
        assert(strcmp(out, params[1]) == 0);
    }
    big_free(&num1);
}

void big_cmp_tester(int idx, char **params) {
    bigint num1, num2;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);

    int cmp = big_cmp(&num1, &num2);

    if (!((cmp == -1 && strcmp(params[2], "-1") == 0) ||
        (cmp == 0 && strcmp(params[2], "0") == 0) ||
        (cmp == 1 && strcmp(params[2], "1") == 0))) {
        printf("case #%d CMP(%s %s) = %d, %s\n", idx, params[0], params[1], cmp, params[2]);
        assert(false /* laziness */);
    }
    
    big_free(&num1);
    big_free(&num2);
}

void big_div_tester(int idx, char **params) {
    (void)idx;
    bigint num1, num2, result, remainder;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&result);
    big_init(&remainder);
    // printf("TRYING case %d %s / %s = %s R%s\n", idx, params[0], params[1], params[2], params[3]);
    big_div(&result, &remainder, &num1, &num2);
    char out1[128];
    char out2[128];
    big_write_string(&result, out1, 128, &olen);
    big_write_string(&remainder, out2, 128, &olen);
    // printf("case #%d %s / %s = %s R%s, %s R%s\n", idx, params[0], params[1], out1, out2, params[2], params[3]);
    assert(strcmp(out1, params[2]) == 0);
    assert(strcmp(out2, params[3]) == 0);
    big_free(&num1);
    big_free(&num2);
    big_free(&result);
    big_free(&remainder);
}

void big_gcd_tester(int idx, char **params) {
    bigint num1, num2, gcd;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&gcd);
    // printf("TRYING case gcd(%s, %s) = %s\n", params[0], params[1], params[2]);
    big_gcd(&gcd, &num1, &num2);
    char out[128];
    big_write_string(&gcd, out, 128, &olen);
    if (strcmp(out, params[2]) != 0) {
        printf("case #%d gcd(%s, %s) = %s, %s \n", idx, params[0], params[1], out, params[2]);
        assert(strcmp(out, params[2]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&gcd);
}

void big_inv_mod_tester(int idx, char **params) {
    bigint num1, num2, inv;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&inv);
    printf("TRYING case inv_mod(%s, %s)\n", params[0], params[1]);
    big_inv_mod(&inv, &num1, &num2);
    char out[128];
    big_write_string(&inv, out, 128, &olen);

    // (5, 3)
    // 3X + k2 = 1
    // 3X = 1 - k2
    // 3 = 1 + 3
    printf("case #%d inv_mod(%s, %s) = %s, %s \n", idx, params[0], params[1], out, params[2]);
    // assert(strcmp(out, params[2]) == 0);
    big_free(&num1);
    big_free(&num2);
    big_free(&inv);
}


void big_is_prime_tester(int idx, char **params) {
    bigint num;
    size_t olen;
    big_init(&num);
    big_read_string(&num, params[0]);

    int result = big_is_prime(&num);

    if (!((result == 0 && params[1] == "0") || (result == 1 && params[1] == "1"))) {
        printf("case #%d is_prime(%s) = %d, %s \n", idx, params[0], result, params[1]);
        assert(false);
    }

    // char out[128];
    // big_write_string(&gcd, out, 128, &olen);
    // assert(strcmp(out, params[2]) == 0);
    big_free(&num);
}

void big_exp_mod_tester(int idx, char **params) {
    bigint num1, num2, num3, mod, RR;
    size_t olen;
    big_init(&num1);
    big_read_string(&num1, params[0]);
    big_init(&num2);
    big_read_string(&num2, params[1]);
    big_init(&num3);
    big_read_string(&num3, params[2]);
    big_init(&mod);
    big_init(&RR);
    printf("TRYING case exp_mod(%s, %s, %s) = %s\n", params[0], params[1], params[2], params[3]);
    big_exp_mod(&mod, &num1, &num2, &num3, &RR);
    char out[128];
    big_write_string(&mod, out, 128, &olen);
    if (strcmp(out, params[3]) != 0) {
        printf("case #%d exp_mod(%s, %s, %s) = %s, %s \n", idx, params[0], params[1], params[2], out, params[3]);
        assert(strcmp(out, params[3]) == 0);
    }
    big_free(&num1);
    big_free(&num2);
    big_free(&num3);
    big_free(&mod);
    big_free(&RR);
}

void big_poly_eval_tester(int idx, char **params) {
    // there exist 13 params
    bigint x, p_of_x;
    big_init(&x);
    big_init(&p_of_x);
    big_read_string(&x, params[0]);
    int k = 1;
    while (strcmp(params[k], "0") == 0) {
        k++;
    }

    int num_coefs = 12 - k; 
    bigint P[num_coefs];
    for (int i = k; i < 12; i++) {
        big_init(&P[i - k]);
        big_read_string(&P[i - k], params[i]);
    }
    big_init(&p_of_x);
    big_eval_polynomial(&p_of_x, P, num_coefs, &x);

    char out[2048];
    size_t olen;
    big_write_string(&p_of_x, out, 2048, &olen);
    if (strcmp(out, params[12]) != 0) {
        printf("case #%d poly_eval (expected: %s actual: %s\n", idx, out, params[12]);
        assert(strcmp(out, params[3]) == 0);
    }

    big_free(&x);
    big_free(&p_of_x);

    for (int i = 0; i < num_coefs; i++) {
        big_free(&P[i]);
    }
    
}

void run_tests(const char *filename, test_func_t *tester, 
                size_t n_cases, size_t n_params) {
    printf("    %s\n", filename);
    test_cases_t *tc = read_test_cases(filename, n_params, n_cases);
    run_test_cases(tc, tester);
    free_test_cases(tc);
}