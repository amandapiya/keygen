#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include "test_utils.h"

const size_t MAX_LEN = 128;
const char SEPARATOR[2] = ",";

test_cases_t *read_test_cases(const char *filename, 
                                size_t num_params,
                                size_t num_cases) {

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char *line = NULL;
    size_t len = 0;

    ssize_t read;

    test_cases_t *tc = malloc(sizeof(test_cases_t));
    tc->params = malloc(num_cases * sizeof(char **));
    tc->num_cases = num_cases;
    tc->num_params = num_params;


    for (size_t i = 0; i < num_cases; i++) {
        tc->params[i] = malloc(num_params * sizeof(char *));
    }

    int case_num = 0; 
    while ((read = getline(&line, &len, f)) != -1 && case_num < (int)num_cases) {
        line[read - 1] = '\0';
        if (read > 0) {
            char *token = strtok(line, SEPARATOR);
            for (size_t par = 0; par < num_params; par++) {
                tc->params[case_num][par] = strdup(token);
                token = strtok(NULL, SEPARATOR);
            }
        }
        case_num++;
    }

    free(line);

    return tc;

}

void print_test_cases(test_cases_t *tc) {
    for (size_t c = 0; c < tc->num_cases; c++) {
        for (size_t p = 0; p < tc->num_params; p++) {
            printf("%s, ", tc->params[c][p]);
            fflush(stdout);
        }
        printf("\n");
    }
}

void free_test_cases(test_cases_t *tc) {
    for (size_t c = 0; c < tc->num_cases; c++) {
        for (size_t p = 0; p < tc->num_params; p++) {
            free(tc->params[c][p]);
        }
        free(tc->params[c]);
    }
    free(tc->params);
    free(tc);
}

void run_test_cases(test_cases_t *tc, test_func_t *test_func) {
    for (size_t i = 0; i < tc->num_cases; i++) {
        test_func((int)i, tc->params[i]);
    }
}