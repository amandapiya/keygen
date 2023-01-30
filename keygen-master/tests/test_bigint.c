#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "bigint.h"
#include "test_utils.h"
#include "test_bigint_utils.c"


int main() {
    
    printf("Running tests:\n");
    
    run_tests("test_cases/kara_split.csv", big_kara_split_tester, 11, 7);
    run_tests("test_cases/big_mul_kara.csv", big_karatsuba_mul_tester, 13, 3);
    run_tests("test_cases/stress_test_mul.csv", big_karatsuba_mul_tester, 1252, 3);
    run_tests("test_cases/long_mul.csv", big_karatsuba_mul_tester, 500, 3);
    
    // run_tests("test_cases/big_shr.csv", big_shr_tester, 500, 3);
    run_tests("test_cases/div_by_3.csv", big_div_by_3_tester, 500, 2);
    
    // run_tests("test_cases/big_read_binary.csv", big_read_binary_tester, 1, 3);
    run_tests("test_cases/big_read_write_binary.csv", big_read_write_binary_tester, 16, 3);
    run_tests("test_cases/stress_test_write_binary.csv", big_read_write_binary_tester, 50, 3);
    run_tests("test_cases/big_read_write.csv", big_read_write_tester, 15, 3);
    run_tests("test_cases/big_read_invalid.csv", big_read_tester_invalid, 2, 1);
    run_tests("test_cases/big_read_write_buff_too_small.csv", big_read_write_buff_too_small_tester, 11, 3);

    run_tests("test_cases/big_sub_zeros.csv", big_sub_tester, 34, 3);
    run_tests("test_cases/big_add.csv", big_add_same_memory_same_tester, 10, 3);
    run_tests("test_cases/stress_test_mul.csv", big_mul_tester, 1252, 3);
    run_tests("test_cases/stress_test_mul.csv", big_tc_mul_tester, 1252, 3); 
    run_tests("test_cases/long_mul.csv", big_tc_mul_tester, 500, 3);
    run_tests("test_cases/long_mul.csv", big_tc_mul_tester, 500, 3);

    run_tests("test_cases/stress_test_add.csv", big_add_tester, 1250, 3);
    run_tests("test_cases/big_add_repeat.csv", big_add_all_memory_same_tester, 5, 3);
    run_tests("test_cases/big_copy.csv", big_copy_tester, 5, 1);
    run_tests("test_cases/big_sub.csv", big_sub_tester, 30, 3);
    run_tests("test_cases/stress_test_sub.csv", big_sub_tester, 1251, 3);
    run_tests("test_cases/big_add.csv", big_add_tester, 11, 3);
    run_tests("test_cases/big_cmp.csv", big_cmp_tester, 11, 3);
    run_tests("test_cases/stress_test_cmp.csv", big_cmp_tester, 582, 3);

    run_tests("test_cases/stress_test_poly_eval.csv", big_poly_eval_tester, 37600, 13); 

    run_tests("test_cases/josh.csv", big_div_tester, 1, 4);
    run_tests("test_cases/stress_test_gcd.csv", big_gcd_tester, 1800, 3);
    run_tests("test_cases/basic_div_pos.csv", big_div_tester, 406, 4);
    run_tests("test_cases/stress_test_inv_mod.csv", big_inv_mod_tester, 500, 3);
    run_tests("test_cases/big_inv_mod.csv", big_inv_mod_tester, 6, 3);

    run_tests("test_cases/basic_div_pos.csv", big_div_tester, 405, 4);
    run_tests("test_cases/stress_test_div.csv", big_div_tester, 1500, 4);
    run_tests("test_cases/stress_test_inv_mod.csv", big_inv_mod_tester, 500, 2);    
    run_tests("test_cases/big_inv_mod.csv", big_inv_mod_tester, 5, 2);
    run_tests("test_cases/big_exp_mod.csv", big_exp_mod_tester, 4, 4);

    run_tests("test_cases/stress_test_exp_mod.csv", big_exp_mod_tester, 50, 4);

    run_tests("test_cases/first_primes.csv", big_is_prime_tester, 9998, 4);
    run_tests("test_cases/random_primes.csv", big_is_prime_tester, 3600, 4);


    printf("All test cases passed!\n");

}

