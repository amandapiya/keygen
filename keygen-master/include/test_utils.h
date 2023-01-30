#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


typedef struct test_cases {
    size_t num_params;
    size_t num_cases;
    char ***params; /*  pointer to num_cases array of string 
                        lists each of which has num_params elements */
} test_cases_t;

typedef void (test_func_t)(int idx, char **params);

test_cases_t *read_test_cases(const char *filename,
                              size_t num_params,
                              size_t num_cases);

void print_test_cases(test_cases_t *tc);

void free_test_cases(test_cases_t *tc);

void run_test_cases(test_cases_t *tc, test_func_t *test_func);

#endif /* TEST_UTILS_H */
