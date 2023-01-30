#include "bigint.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <math.h>
#include <ctype.h>

static bool LOG_DEBUG = false;

void big_print(bigint *X){
    char buf[X->num_limbs * 16 + 5]; 
    size_t olen;
    big_write_string(X, buf, X->num_limbs * 16 + 5, &olen);
    printf("%s\n", buf);
}

// helper function to change the number of limbs of X if X is too small. 
static int resize_limbs(bigint *X, size_t n_limbs) {
    if (n_limbs == 0) {
        if (X->data != NULL) {
            free(X->data);
        }
        X->data = NULL;
        X->num_limbs = 0;
        X->signum = 0;
        return 0;
    }
    if (X->num_limbs < n_limbs) {
        free(X->data);
        X->data = malloc(sizeof(big_uint) * n_limbs);
        if (X->data == NULL) {
            return ERR_BIGINT_ALLOC_FAILED;
        }
    }
    X->num_limbs = n_limbs; 
    return 0;
}


static int resize_limbs_no_free_X_data(bigint *X, size_t n_limbs) {
    // printf("xnl %d xbuf %p\n", X->num_limbs, X->data);
    if (n_limbs == 0) {
        if (X->num_limbs > 0 && X->data != NULL) {
            // printf("free A\n");
            free(X->data);
        }
        X->data = NULL;
        X->num_limbs = 0;
        X->signum = 0;
        return 0;
    } else if (X->num_limbs < n_limbs) {
        if (X->num_limbs > 0 && X->data != NULL) {
            // printf("X->data = %p\n", &(X->data));
            // printf("free B\n");
            free(X->data);
        }
        X->data = calloc(sizeof(big_uint), n_limbs);
        // printf("rlnfxd %p\n", X->data);
        if (X->data == NULL) {
            return ERR_BIGINT_ALLOC_FAILED;
        }
    }
    X->num_limbs = n_limbs; 
    return 0;
}

// removes limbs that are 0, to allow other functions to work 
// properly without adding additional limbs
static int trim_limbs(bigint *X) {
    size_t i = 0;
    for (i = 0; i < X->num_limbs; i++) {
        if (X->data[i] != 0) {
            break;
        }
    }
    int remaining_limbs = X->num_limbs - i;
    big_uint* new_data = malloc(sizeof(bigint) * remaining_limbs);
    for (size_t j = i; j < X->num_limbs; j++) {
        new_data[j - i] = X->data[j];
    }
    free(X->data);
    X->data = new_data;
    X->num_limbs = remaining_limbs;
    if (X->num_limbs == 0){
        X->signum = 0;
    }
    return i;
}

static int check_zero(bigint *X) {
    size_t i = 0; 
    for (i = 0; i < X->num_limbs; i++) {
        if (X->data[i] != 0) {
            return 0;
        }
    }
    
    if (X->num_limbs > 0) {
        free(X->data);
    }
    *X = BIG_ZERO;
    return i; 
}

static bool _addition_overflows(big_uint x1, big_uint x2) {
    return ULONG_MAX - x2 < x1; // <=> ULONG_MAX < x1 + x2
}

void big_init(bigint *X) {
    *X = BIG_ZERO;
}

void big_free(bigint *X) {
    if (X == NULL) {
        return;
    }
    // printf("free %p\n", X->data);
    // printf("free %p\n", &X);
    free(X->data);
}

int big_copy(bigint *X, const bigint *Y) {
    X->signum = Y->signum;
    int err = resize_limbs_no_free_X_data(X, Y->num_limbs);
    if (err != 0){
        return err;
    }
    if (Y->num_limbs > 0) {
        // printf("xnl %d\n", X->num_limbs);
        // printf("ynl %d\n", Y->num_limbs);
        volatile char* aa = X->data;
        volatile char* bb = Y->data;
    // printf("X buf addr %p\n", X->data);
    // printf("Y buf addr %p\n", Y->data);

        *(bb + Y->num_limbs * sizeof(big_uint) - 1);
        *(aa + Y->num_limbs * sizeof(big_uint) - 1);
        memcpy(X->data, Y->data, Y->num_limbs * sizeof(big_uint));
    } else {
        if (X->data != NULL){
            // printf("free in big_copy\n");
            free(X->data);
            X->data = NULL;
        }
    }
    return 0;
}

// Does NOT account for the negative sign
size_t big_bitlen(const bigint *X) {
    if (X->num_limbs == 0) {
        return 0;
    }
    big_uint most_sig = X->data[0];
    int idx = 0; 
    while (most_sig == 0 && idx < (int)X->num_limbs - 1) {
        most_sig = X->data[++idx];
    }

    int shift = 63;
    while (most_sig >> shift == 0) {
        shift--;
    }
    return sizeof(big_uint) * (X->num_limbs - 1) * 8 + (shift + 1);
}

// size_t big_bitlen(const bigint *X) {
//     return big_bitlen_no_sign(X) + (X->signum == -1 ? 8 : 0);
// }

size_t big_size(const bigint *X) {
    return (size_t) ceil(big_bitlen(X) / 8.0);
    // return sizeof(*X) + X->num_limbs * sizeof(big_uint);
}

int big_set_nonzero(bigint *X, big_uint limb) {
    if (limb <= 0) {
        return ERR_BIGINT_BAD_INPUT_DATA;
    }
    if (X->num_limbs < 1) {
        free(X->data);
        X->data = malloc(sizeof(big_uint));
        if (X->data == NULL) {
            return ERR_BIGINT_ALLOC_FAILED;
        }
        X->num_limbs = 1;
    }
    X->num_limbs = 1;
    X->data[0] = limb; 
    X->signum = 1; 
    return 0;
}

int big_read_string(bigint *X, const char *s) {
    size_t CHARS_PER_LIMB = 16;

    // check for invalid hex digit 
    for (int i=0; i < strlen(s); i++) {
        if (!isxdigit(s[i]) && s[i] != '-') {
            // printf("NOT DIGIT: %c\n", s[i]);
            return ERR_BIGINT_BAD_INPUT_DATA;
        }
    }

    char *trimmed_s = (char *) s;
    // read minus sign, if there is one
    if (trimmed_s[0] == '-') {
        trimmed_s += 1;
        X->signum = -1;
    } else {
        X->signum = 1;
    }

    // remove leading 0s
    // problem if the leading 0s fall across a boundary
    // between two limbs, we don't want to mistakenly allocate
    // an extra one
    while (trimmed_s[0] == '0') {
        trimmed_s += 1;
    }

    size_t len = strlen(trimmed_s);

    size_t n_limbs = len / CHARS_PER_LIMB;
    if (len % CHARS_PER_LIMB != 0) {
        n_limbs += 1;
    }

    int res = resize_limbs(X, n_limbs);
    if (res != 0){
        return res;
    }

    char *new_string = malloc(CHARS_PER_LIMB * n_limbs + 1);
    size_t padlen = CHARS_PER_LIMB * n_limbs - len;
    memset(new_string, '0', padlen);
    strcpy(new_string + padlen, trimmed_s);

    // printf("stored= ");
    for (size_t i = 0; i < n_limbs; i++) {
        char *this_string = malloc(17 * sizeof(char));

        strncpy(this_string, new_string + 16 * i, 16);
        this_string[16] = '\0';
        errno = 0;
        X->data[i] = strtoul(this_string, NULL, 16);

        // uint8_t biggest_uint8 = 255;
        // uint64_t biggest_uint64 = 18446744073709551615;
        // printf("%lx", biggest_uint64 & ((uint64_t *) X->data)[i]);

        // When the string can't be converted to a number: 
        if (((X->data[i] == 0 || X->data[i] == ULONG_MAX) && (errno == ERANGE || errno == EINVAL || errno == EDOM))) {
            // printf("bad_input_data\n");
            return ERR_BIGINT_BAD_INPUT_DATA;
        }
        free(this_string);
    }
    // printf("\n");
    X->num_limbs = n_limbs;
    free(new_string);    
    return 0;
}

int big_write_string(const bigint *X,
                     char *buf, size_t buflen, size_t *olen) {
    memset(buf, 0, buflen);   
    size_t curr_buflen = (size_t) (ceil(big_bitlen(X) / 4.0) + 1);   // +1 accounts for null

    // if (X->data[0] >> 4 == 0) {     // accounts for an extra halfbit 
    //     curr_buflen -= 1;
    // }
    if (curr_buflen == 1) {     // if 0
        curr_buflen = 2;
    }
    if (X->signum == -1) {
        curr_buflen += 1;
    }
    if (curr_buflen > buflen) {
        *olen = (size_t) (curr_buflen);
        return ERR_BIGINT_ALLOC_FAILED;
    }
    *olen = curr_buflen; 

    // bool should_return_err = false;

    if (X->signum == -1) {
        strcat(buf, "-");
    }
    char *str_result = calloc(17, sizeof(char));
    int first_nonzero_idx = 0;
    bool first_nonzero = false;
    for (size_t i = 0; i < X->num_limbs; i++) {
        if (X->data[i] == 0 && !first_nonzero) {
            continue;
        }
        if (!first_nonzero) {
            first_nonzero_idx = i;
        }
        first_nonzero = true;
        big_uint limb = X->data[i];
        memset(str_result, '0', 16);
        // each accounts for the null terminator that sprintf adds
        char each[2];
        for (int j = 0; j < 16; j++) {
            sprintf(each, "%llx", limb % 16);
            str_result[16 - j - 1] = each[0];
            limb /= 16;

        }

        char *trim_result = str_result;
        // Remove left padding for the most significant limb only
        if (i == first_nonzero_idx) {
            while (trim_result[0] == '0') {
                trim_result = ((void *)trim_result) + 1;
            }
        }
        strcat(buf, trim_result);

    }
    free(str_result);

    if (strlen(buf) == 0) {
        strcpy(buf, "0");
    }
    
    return 0;
}


int big_read_binary(bigint *X, const uint8_t *buf, size_t buflen) {
    uint8_t *trimmed_buf = (uint8_t *) buf;
    size_t newlen = buflen;

    // handle the case where the binary is just 0 (it will get trimmed off)
    // printf("%zu\n", *trimmed_buf);
    // int len_buf =
    // if ()

    printf("trimmed: %d\n", *trimmed_buf);

    while (trimmed_buf[0] == 0) {
        trimmed_buf++;
        newlen --;
    }

    if (newlen == 0 && buflen > 0) {
        big_copy(X, &BIG_ZERO);
        return 0;
    }

    size_t n_limbs = newlen / 8;
    if (newlen % 8 != 0) {
        n_limbs += 1;
    }
    
    int res = resize_limbs(X, n_limbs);
    if (res != 0){
        return res;
    }

    X->data[0] = 0;
    printf("n_limbs: %zu, trimmed: %zu\n", n_limbs, trimmed_buf[0]);
    for (size_t i = 8 - (newlen % 8); i < 8 * n_limbs; i++) {
        uint8_t *limb = (uint8_t *) X->data[i / 8];   
        limb[i % 8] = trimmed_buf[i - 8 + (newlen % 8)];
    }
    return 0;
}

long decimalToBinary(big_uint decimal) {
    long binary = 0;
    int remainder = 1;
    int temp = 1;

    while (decimal != 0) {
        remainder = decimal % 2;
        decimal /= 2;
        binary += remainder * temp;
        temp *= 10;
    }
    return binary;
}

int big_write_binary(const bigint *X, uint8_t *buf, size_t buflen) {
    memset(buf, 0, buflen);

    size_t n_bytes = big_bitlen(X) / 8 + (big_bitlen(X) % 8 == 0 ? 0 : 1);
    if (n_bytes > buflen) {
        return ERR_BIGINT_BUFFER_TOO_SMALL;
    }

    // handle the 0 case
    if (X->data == NULL && X->num_limbs == 0) {
        return 0;
    }

    // accounts for left padding buf
    size_t padlen = buflen - (n_bytes);   // accounts for neg in n_bytes now
    size_t idx = padlen;


    for (size_t i = 0; i < X->num_limbs; i++) {        
        uint64_t biggest_uint64 = 18446744073709551615;
        uint64_t temp = biggest_uint64 & X->data[i];

        for (int j = 7; j >= 0; j--) {
            uint8_t binary = temp >> (8 * j);
            // printf("binary = %02x\n", binary);
            assert(idx < buflen);

            if (i == 0 && binary == 0) {       
                // take out leading 0s in the first limb
            } else {
                buf[idx] = binary;
                idx++;
            }
            // printf("binary = %02x\n", binary);
        }
    }

    return 0;
}

int big_add_notrim(bigint *X, const bigint *A, const bigint *B) {
    if (A->num_limbs == 0){
        big_copy(X, B);
        return 0;
    }

    if (B->num_limbs == 0){
        big_copy(X, A);
        return 0;
    }

    if (A->signum != B->signum) {
        bigint flipped;
        big_init(&flipped);
        if (B->signum == -1) {
            // A + B == A - (-B), if B < 0
            big_copy(&flipped, B);
            flipped.signum = -flipped.signum;
            big_sub(X, A, &flipped);
        } else {
            // A + B == B - (-A), if A < 0
            big_copy(&flipped, A);
            flipped.signum = -flipped.signum;
            big_sub(X, B, &flipped);
        }
        big_free(&flipped);
        return 0;
    }

    const bigint *longer = A;
    const bigint *shorter = B;
    if (B->num_limbs > A->num_limbs) {
        longer = B;
        shorter = A;
    }

    if (shorter->num_limbs == 0) {
        big_copy(X, longer);
        return 0;
    }

    // prioritizing assuming if there is an extra limb 
    // than to caluclate if there will be one -> 
    // algo: starting at highest base + compare at same level
    // (l[0] / MAX) + (s[0] / MAX) >= 1 (if = 1-1/MAX, recurse through lower limbs)
    size_t num_limbs = longer->num_limbs + 1;
    int offset = longer->num_limbs - shorter->num_limbs;
    big_uint *new_data = malloc(sizeof(big_uint) * num_limbs);
    int c = 0;
    big_udbl calc = 0;

    for (int i = (int)shorter->num_limbs - 1; i >= 0; i--) {
        calc = (big_udbl) longer->data[i + offset] + (big_udbl) shorter->data[i] + c;
        new_data[i + offset + 1] = (big_uint) calc & ULLONG_MAX;
        c = (calc >> 64) & 1;
    }

    for (int i = offset - 1; i >= 0; i--) {
        calc = (big_udbl) longer->data[i] + c;
        new_data[i + 1] = (big_uint) calc & ULLONG_MAX;
        c = (calc >> 64) & 1;
    }

    free(X->data);

    if (c == 1) {
        new_data[0] = 1;
        X->data = new_data;
    } else {
        // no overflow
        num_limbs -= 1;
        big_uint *no_flow = malloc(sizeof(big_uint) * num_limbs);
        memcpy(no_flow, new_data+1, sizeof(big_uint) * num_limbs);
        free(new_data);
        X->data = no_flow;
    }

    X->num_limbs = num_limbs;
    X->signum = A->signum;
    check_zero(X);

    return 0;
}

int big_add(bigint *X, const bigint *A, const bigint *B) {
    int result = big_add_notrim(X, A, B);
    if (result == 0) {
        trim_limbs(X);
    }
    return result;
}

int big_sub_notrim(bigint *X, const bigint *A, const bigint *B) {

    if (A->signum == 0) {
        big_copy(X, B);
        X->signum = - B->signum;
        trim_limbs(X);
        return 0;
    }

    if (B->signum == 0) {
        big_copy(X, A);
        trim_limbs(X);
        return 0;
    }

    if (A->signum != B->signum) {
        bigint flipped;
        big_init(&flipped);
        big_copy(&flipped, B);
        flipped.signum = -flipped.signum;

        // A - B == A + (-B)
        big_add(X, A, &flipped);
        big_free(&flipped);
        return 0;
    }

    size_t first_A = 0;

    while (A->data[first_A] == 0 && first_A < (int)A->num_limbs - 1) {
        first_A++;
    }

    size_t first_B = 0;
    
    while (B->data[first_B] == 0 && first_B < (int)B->num_limbs - 1) {
        first_B++;
    }

    size_t len_A = A->num_limbs - first_A;
    size_t len_B = B->num_limbs - first_B;

    const bigint *left = A; 
    const bigint *right = B;
    X->signum = left->signum;

    if (len_A <= len_B){
        bool change = len_B > len_A;
        int i = 0;
        while (!change && i < len_A){
            if (B->data[first_B + i] > A->data[first_A + i]){
                change = true;
            } else if (A->data[first_A + i] > B->data[first_B + i]){
                break;
            }
            i++;
        }
        if (change){
            left = B;
            right = A;
            X->signum = -1 * left->signum;
        }
    }

    big_uint *new_data = malloc(left->num_limbs * sizeof(big_uint));
    if (new_data == NULL) {
        return ERR_BIGINT_ALLOC_FAILED;
    }
    int offset = left->num_limbs - right->num_limbs;
    int c = 0;
    for (int i = left->num_limbs - 1; i >= 0; i--) {
        big_sdbl calc = (big_sdbl) left->data[i] + c;
        if (i >= offset) {
            calc -= (big_sdbl) right->data[i - offset];
        }
        new_data[i] = (big_uint) (calc & ULLONG_MAX);
        c = calc < 0 ? -1 : 0;
    }

    free(X->data);
    X->data = new_data;
    X->num_limbs = left->num_limbs;
    check_zero(X);

    return 0;
}

int big_sub(bigint *X, const bigint *A, const bigint *B) {
    int result = big_sub_notrim(X, A, B);
    if (result == 0) {
        trim_limbs(X);
    }
    return result;
}

int big_cmp(const bigint *x, const bigint *y) {
    if ((x->num_limbs == 0 && y->num_limbs == 0) || (x->signum == 0 && y->signum == 0)){
        return 0;
    }

    if (x->num_limbs == 0 || x->signum == 0){
        return y->signum == 1 ? -1 : 1;
    }

    if (y->num_limbs == 0 || y->signum == 0){
        return x->signum == -1 ? -1 : 1;
    }

    if (x->signum != y->signum){
        // must check if both are 0 before sign
        return x->signum == -1 ? -1 : 1;
    }

    if (x->num_limbs != y->num_limbs){
        // more limbs = higher absolute value
        return ((x->signum == 1 && x->num_limbs < y->num_limbs) || (x->signum == -1 && x->num_limbs > y->num_limbs)) ? -1 : 1;
    }

    for (size_t i = 0; i < x->num_limbs; i++) {
        if (x->data[i] > y->data[i]) {
            return x->signum;
        } else if (y->data[i] > x->data[i]) {
            return -x->signum;
        } 
    }
    return 0;
}


int big_mul(bigint *X, const bigint *A, const bigint *B) {

    // X will take at most (A->num_limbs + B->num_limbs + 2) space
    big_uint *new_data = calloc((A->num_limbs + B->num_limbs), sizeof(big_uint));
    if (new_data == NULL){
        return ERR_BIGINT_ALLOC_FAILED;
    }

    big_udbl uv = 0;

    size_t new_size = A->num_limbs + B->num_limbs;
    for (int B_idx = B->num_limbs - 1; B_idx >= 0; B_idx--) {
        big_uint carry = 0;

        for (int A_idx = A->num_limbs - 1; A_idx >= 0; A_idx--) {

            int i_plus_j = (A->num_limbs - A_idx - 1) + (B->num_limbs - B_idx - 1);

            uv = new_data[new_size - 1 - i_plus_j] + ((big_udbl)A->data[A_idx] * (big_udbl)B->data[B_idx]) + carry;

            new_data[new_size - 1 - i_plus_j] = (big_uint)(uv & ULONG_MAX); // ones digit
            carry = (big_uint)(uv >> 64);
        } 
        // n + 1 = A->num_limbs
        // i = B->num_limbs - B_idx 
        // n + 1 + i = A->num_limbs + B->num_limbs - B_idx
        // Convert to an index: 
        new_data[B_idx] = (big_uint) ((uv >> 64) & ULONG_MAX); // tens digit
    }
    
    X->signum = A->signum * B->signum;
    X->num_limbs = new_size;        
    if (X->data != NULL) {
        free(X->data);
    }
    X->data = new_data;
    trim_limbs(X);
    return 0;
}

int big_eval_polynomial(bigint *Y, bigint *A, size_t len_A, bigint *X) {
    /* Zero out result */
    big_copy(Y, &BIG_ZERO);
    for (size_t i = 0; i < len_A; i++) {
        /* Horner's rule evaluation */
        big_add(Y, Y, &A[i]);
        // big_print(Y);
        if (i != len_A - 1) {
            big_mul(Y, Y, X);
        }
    }
    return 0;
}

int big_shr(bigint *X, const bigint *A, size_t shift) {
    int limb_shift = shift / 64;
    size_t bit_shift = shift % 64;

    if (X->signum == 0) {
        return 0;
    }

    // assert(X->num_limbs >= A->num_limbs - limb_shift);
    uint64_t *new_data = calloc(A->num_limbs, sizeof(big_uint));

    for (int i = A->num_limbs - 1; i >= (int)limb_shift; i--) {
        new_data[i] = A->data[i - limb_shift] >> bit_shift;
        if (i - limb_shift - 1 >= 0) {
            uint64_t prev_limb = A->data[i - limb_shift - 1];
            prev_limb &= (1ULL << bit_shift) - 1;
            new_data[i] |= prev_limb << (64 - bit_shift);
        }
    }

    for (int i = limb_shift - 1; i >= 0; i--) {
        new_data[i] = 0;
    }
    if (X->data != NULL) {
        free(X->data);
    }
    X->data = new_data;
    X->num_limbs = A->num_limbs;
    X->signum = A->signum;
    check_zero(X);
}

int big_fast_divide_by_3(bigint *X, const bigint *A) {
    if (A->signum == 0) {
        if (X->data != NULL) {
            free(X->data);
        }
        *X = BIG_ZERO;
        return 0;
    }

    X->signum = A->signum;
    resize_limbs(X, A->num_limbs);

    uint64_t borrow = 0;
    for (int i = A->num_limbs - 1; i >= 0; i--) {
        big_udbl w = (big_udbl)A->data[i] - borrow;
        borrow = (borrow > A->data[i]) ? 1 : 0;
        uint64_t new_limb = (uint64_t)w * 0xAAAAAAAAAAAAAAABLL;
        if ((uint64_t) new_limb > 0x5555555555555555) {
            borrow++;
        }
        if ((uint64_t) new_limb > 0xAAAAAAAAAAAAAAABLL) {
            borrow++;
        }
        X->data[i] = new_limb;
    }
}

int big_quad_divide_by_3(bigint *X, const bigint *A) {
    if (A->signum == 0) {
        if (X->data != NULL) {
            free(X->data);
        }
        *X = BIG_ZERO;
        return 0;
    }

    bigint mod_inv_3; 
    big_init(&mod_inv_3);
    mod_inv_3.num_limbs = A->num_limbs;
    mod_inv_3.signum = 1;
    mod_inv_3.data = calloc(A->num_limbs, sizeof(big_uint));
    memset(mod_inv_3.data, 0x55, A->num_limbs * sizeof(big_uint));
    mod_inv_3.data[A->num_limbs - 1] += 1; 

    big_mul(X, A, &mod_inv_3);
    big_shr(X, X, 64 * mod_inv_3.num_limbs);
    big_free(&mod_inv_3);
}

/* Divides A by 3 and stores the result in X */
int big_fast_divide_by_3_by_approx(bigint *X, const bigint *A) {
    /*
     * 1/3 = 1/4 * (1 + 2^-2)(1 + 2^-4)(1 + 2^-8)(1 + 2^-16)...(1 + 2^-i)
     */
    
    // First, right-shift A by 2 
    printf("After right shift by 2\n");
    big_print(X);

    if (A->num_limbs == 0) {
        if (X->data != NULL) {
            free(X->data);
        }
        *X = BIG_ZERO;
        return 0;
    }

    int i = 2;
    bigint temp;
    big_init(&temp);
    temp.data = calloc(X->num_limbs, sizeof(big_uint));
    big_copy(X, A);
    printf("start X = ");
    big_print(X);
    while (i <= A->num_limbs * 64) {
        // Then, X <- X + (X >> 2^2^i+1) for i = 0, 1, 2, ..
        big_shr(&temp, X, i);
        big_add(X, X, &temp);

        printf("i = %d:\n", i);
        printf("temp = ");
        big_print(&temp);
        printf("X = ");
        big_print(X);
        printf("\n");
        i *= 2; 
    }
    int round = X->data[X->num_limbs - 1] % 4;
    big_shr(X, X, 2);
    printf("round = %d\n", round);
    if (round == 3) {
        printf("ROUNDING BITCH\n");
        big_set_nonzero(&temp, 1);
        big_add(X, X, &temp);
    }
    big_free(&temp);
    X->signum = A->signum;
    check_zero(X);

}

// Split the two bigint numbers about the middle
int karatsuba_low_high(bigint *low, bigint *high, size_t m2, bigint *A) {
    if (A->num_limbs > m2){
        size_t split_idx = A->num_limbs - m2;

        low->signum = 1;
        high->signum = 1;
        low->num_limbs = m2;
        high->num_limbs = A->num_limbs - m2;
        big_uint *data_low = malloc(sizeof(big_uint) * low->num_limbs);
        big_uint *data_high = malloc(sizeof(big_uint) * high->num_limbs);
        // printf("lh %p %p\n", data_low, data_high);
        if (data_low == NULL || data_high == NULL){
            return ERR_BIGINT_ALLOC_FAILED;
        }

        int idx = 0;
        for (size_t i = split_idx; i < A->num_limbs; i++) {
            // THIS LINE??? 
            data_low[idx] = A->data[i];
            // printf("%zu", data_low[idx]);
            idx++;
        }
        low->data = data_low;

        idx = 0;
        for (size_t i = 0; i < split_idx; i++) {
            data_high[idx] = A->data[i];
            // printf("data_high[0] = %d\n", data_high[idx]);
            idx++;
        }
        high->data = data_high;
        
    } else {
        // printf("high = %p\n", &high);
        // printf("low = %p\n", &low);
        // printf("A = %p\n", &A);
        // printf("big_copy\n");
        big_copy(high, &BIG_ZERO);
        big_copy(low, A);
    }
    return 0;
 }

// The actual karatsuba algo
int big_mul_karatsuba_helper(bigint *X, const bigint *A, const bigint *B) {
    // printf("A->num_limbs = %d\n", A->num_limbs);
    // printf("B->num_limbs = %d\n", B->num_limbs);
    if (A->num_limbs == 0 || B->num_limbs == 0) {
        big_copy(X, &BIG_ZERO);
        return 0;
    }
    if (A->num_limbs == 1 || B->num_limbs == 1) {
        big_mul(X, A, B); // must be different mul
        return 0;
    }

    size_t m = A->num_limbs > B->num_limbs ? A->num_limbs : B->num_limbs;
    size_t m2 = ceil((double) m / 2.0);
    // printf("m2 = %zu\n", m2);

    // Split A
    bigint highA;
    big_init(&highA);
    bigint lowA;
    big_init(&lowA);
    // printf("kara low high A\n");
    int err = karatsuba_low_high(&lowA, &highA, m2, A);
    if (err != 0){
        return err;
    }

    // Split B
    bigint highB;
    big_init(&highB);
    bigint lowB;
    big_init(&lowB);
    // printf("hbnl %d hbbuf %p\n", highB.num_limbs, highB.data);
    // printf("lbnl %d lbbuf %p\n", lowB.num_limbs, lowB.data);
    // printf("kara low high B\n");
    // printf("B buf addr b4lh %p\n", B->data);
    err = karatsuba_low_high(&lowB, &highB, m2, B);
    // printf("kara low high C\n");
    if (err != 0){
        return err;
    }
    
    bigint z0;
    big_init(&z0);
    bigint z1a;
    big_init(&z1a);
    bigint z1b;
    big_init(&z1b);
    bigint z1;
    big_init(&z1);
    bigint z2;
    big_init(&z2);
    // printf("kara helper A\n");
    big_mul_karatsuba_helper(&z0, &lowA, &lowB);
    big_mul_karatsuba_helper(&z2, &highA, &highB);
    big_add(&z1a, &lowA, &highA);
    big_add(&z1b, &lowB, &highB);
    big_mul_karatsuba_helper(&z1, &z1a, &z1b);
    // printf("kara helper B\n");

    // z2 * 2 ^ 64 ^ (2 * m2) + 
    big_two_to_pwr(X, 64 * 2 * m2);
    big_mul(X, X, &z2); // any mul

    // (z1 - z2 - z0) * 2 ^ 64 ^ m2 + 
    big_sub(&z1a, &z1, &z2);
    big_sub(&z1a, &z1a, &z0);
    big_two_to_pwr(&z1b, 64 * m2);
    big_mul(&z1a, &z1a, &z1b); // any mul
    big_add(X, X, &z1a);
    
    // + z0
    big_add(X, X, &z0);
    // printf(" end of helper free\n");
    big_free(&lowA);
    big_free(&highA);
    // printf("lowB = %p\n", &lowB);
    big_free(&lowB);
    big_free(&highB);
    big_free(&z0);
    big_free(&z1);
    big_free(&z1a);
    big_free(&z1b);
    big_free(&z2);

    // size_t olen5;
    // char out5[256];
    // big_write_string(X, out5, 256, &olen5);
    // printf("X = %s\n", out5);

    return 0;
}

// This outer layer is to not have to deal with signs
int big_mul_karatsuba(bigint *X, const bigint *A, const bigint *B){
    trim_limbs(A);
    trim_limbs(B);
    int a = A->signum * B->signum;
    // printf("B buf addr %p\n", B->data);
    int err = big_mul_karatsuba_helper(X, A, B);
    if (err != 0){
        return err;
    }
    X->signum = a;
    trim_limbs(X);
    return 0;
}

int big_mul_tc(bigint *X, const bigint *A, const bigint *B) {
    /* For now, only implement Toom-3 */
    /* Possible extension would be to implement generalized Toom-k */
    int k = 3;

    // First, find i such that the max number of digits of A and B
    // in base b^i is at most 3. 

    if (A->num_limbs == 0 || B->num_limbs == 0) {
        // printf("Short circuit\n");
        big_copy(X, &BIG_ZERO);
        return 0; 
    }

    size_t len_A = A->num_limbs;
    size_t len_B = B->num_limbs;

    if (LOG_DEBUG) {
        printf("len_A: %zu\n", len_A);
        printf("len_B: %zu\n", len_B);
    }

    size_t A_i = floor((len_A - 1) / k) + 1;
    size_t B_i = floor((len_B - 1) / k) + 1;

    size_t len_segment = A_i;
    
    if (B_i > A_i) {
        len_segment = B_i;
    }

    // printf("len_segment: %zu\n", len_segment);

    bigint A_base[k];
    bigint B_base[k]; 

    /* Start by grouping the limbs into 3 limbs of length len_segment */ 
    for (int i = 0; i < k; i++) {
        // printf("k-i = %d\n", k-i);
        big_init(&A_base[k-i-1]);
        big_init(&B_base[k-i-1]);

        // num_limbs = 3, len_segment = 1
        // would give us A_base[3 - 0 - 1] = num_limbs - 1

        int min_A_idx = A->num_limbs - (i + 1) * len_segment;
        if (min_A_idx < 0) {
            if ((int)len_segment + min_A_idx > 0) { // there's part of the number we want
                A_base[k - i - 1].num_limbs = len_segment + min_A_idx;
                A_base[k - i - 1].data = &A->data[0];
            } else {
                // printf("adawdasd\n");
                A_base[k - i - 1] = BIG_ZERO;
            }
        } else {
            A_base[k - i - 1].num_limbs = len_segment;
            A_base[k - i - 1].data = &A->data[min_A_idx];
        }

        int min_B_idx = B->num_limbs - (i + 1) * len_segment;
        if (min_B_idx < 0) {
            if ((int)len_segment + min_B_idx > 0) { // there's part of the number we want
                B_base[k - i - 1].num_limbs = len_segment + min_B_idx;
                B_base[k - i - 1].data = &B->data[0];
            } else {
                B_base[k - i - 1] = BIG_ZERO;
            }
        } else {
            B_base[k - i - 1].num_limbs = len_segment;
            B_base[k - i - 1].data = &B->data[min_B_idx];
        }

        A_base[k-i-1].signum = 1;
        B_base[k-i-1].signum = 1;

        // trim_limbs(&A_base[k-i-1]);
        // trim_limbs(&B_base[k-i-1]);
        // printf("A[%d] = ", k-i-1);
        // big_print(&A_base[k-i-1]);
        // printf("B[%d] = ", k-i-1);
        // big_print(&B_base[k-i-1]);
        // printf("sig %d\n", A_base[k-i-1].signum);
        // printf("sig %d\n", B_base[k-i-1].signum);
    }

    /* Now each is a k-array of 3 "digits" representing A and B 
        in base 2^64^len_segment. Wild! */
    
    // TODO: Assert this representation is correct?

    assert(k == 3);
    int points[] = {0, 1, -1, -2}; 
    bigint ab_pts[5];
    bigint a_i, b_i;
    big_init(&a_i);
    big_init(&b_i);
    for (int i = 0; i < 4; i++) {
        big_init(&ab_pts[i]);

        bigint point;
        big_init(&point);
        big_set_nonzero(&point, abs(points[i]));
        
        if (points[i] < 0) {
            point.signum = -1;
        } else {
            point.signum = 1;
        }
        // printf("i= %d\n", i);

        big_eval_polynomial(&a_i, A_base, k, &point);
        big_eval_polynomial(&b_i, B_base, k, &point);

        trim_limbs(&a_i);
        trim_limbs(&b_i);

        if (LOG_DEBUG) {
            printf("points[i] = %d\n", points[i]);
            printf("A(%d) = ", points[i]);
            big_print(&a_i);
            printf("B(%d) = ", points[i]);
            big_print(&b_i);
        }

        // FIXME: Should eventually become a recursive call to toom_cook once
        // the algorithm is in a working state
        // if (a_i.num_limbs > 1 || b_i.num_limbs > 1) {
        //     big_mul_tc(&ab_pts[i], &a_i, &b_i);
        // } else {
        big_mul(&ab_pts[i], &a_i, &b_i);
        // }

        // printf("AB(%d) = ", points[i]);
        // big_print(&ab_pts[i]);

        big_free(&point);
    }

    /* The fifth point is the multiplication of the polynomials evaluated
        "at infinity", which is the coefficient of the highest degree. */

    big_init(&ab_pts[4]);
    big_mul(&ab_pts[4], &A_base[0], &B_base[0]);

    // printf("AB(%d) = ", 4);
    // big_print(&ab_pts[4]);
    
    /* Bodrato (2007) http://www.bodrato.it/papers/#WAIFI2007 has a way
        to compute the inverse of the vandermonde matrix for q = 3 without 
        having to implement matrix multiplication & inversion. So we'll 
        use this for now. */

    /* This is nice because the vandermonde matrix inversion is known for
        common low values of c. Later, we will add the ability to
        support general k */

    bigint ab_coefs[5];
    size_t max_length = 0;
    for (int i = 0; i < 5; i++) {
        big_init(&ab_coefs[i]);
        if (ab_pts[i].num_limbs > max_length) {
            max_length = ab_pts[i].num_limbs;
        }
    }
    max_length += 2;

    for (int i = 1; i < 4; i++) {
        ab_coefs[i].data = calloc(max_length, sizeof(uint64_t));
    }

    bigint TWO;
    big_init(&TWO);
    big_set_nonzero(&TWO, 2UL);

    bigint THREE;
    big_init(&THREE);
    big_set_nonzero(&THREE, 3UL);

    bigint FOUR;
    big_init(&FOUR);
    big_set_nonzero(&FOUR, 4UL);

    /* (ab)_0 <- ab(0) */
    big_copy(&ab_coefs[0], &ab_pts[0]);

    /* (ab)_4 <- ab(\infty) */ 
    big_copy(&ab_coefs[4], &ab_pts[4]);

    /* (ab)_3 <- (ab(-2) - ab(1)) / 3 */
    big_sub(&ab_coefs[3], &ab_pts[3], &ab_pts[1]);
    big_fast_divide_by_3(&ab_coefs[3], &ab_coefs[3]);
    
    // printf("step (ab)_3 = ");
    // big_print(&ab_coefs[3]);

    /* (ab)_1 <- (ab(1) - ab(-1)) / 2 */
    big_sub(&ab_coefs[1], &ab_pts[1], &ab_pts[2]);
    big_shr(&ab_coefs[1], &ab_coefs[1], 1);
    // big_div(&ab_coefs[1], NULL, &ab_coefs[1], &TWO);
    // printf("step (ab)_1 = ");
    // big_print(&ab_coefs[1]);

    /* (ab)_2 <- ab(-1) - ab(0) */
    big_sub(&ab_coefs[2], &ab_pts[2], &ab_pts[0]);
    // printf("step (ab)_2 = ");
    // big_print(&ab_coefs[2]);

    /* (ab)_3 <- (ab_2 - ab_3) / 2 + 2 * ab(inf) 
                = 2[ (ab_2 - ab_3) / 4 + ab(inf)] avoids temp variable */
    bigint temp; // will be set to 2 * ab(inf)
    big_init(&temp);
    big_sub(&ab_coefs[3], &ab_coefs[2], &ab_coefs[3]);
    big_shr(&ab_coefs[3], &ab_coefs[3], 1);
    big_mul(&temp, &ab_coefs[4], &TWO); 
    big_add(&ab_coefs[3], &ab_coefs[3], &temp);
    big_free(&temp);

    // printf("step (ab)_3 = ");
    // big_print(&ab_coefs[3]);

    /* ab_2 <- ab_2 + ab_1 - ab_4 */
    big_add(&ab_coefs[2], &ab_coefs[2], &ab_coefs[1]);
    big_sub(&ab_coefs[2], &ab_coefs[2], &ab_coefs[4]);

    /* ab_1 <- ab_1 - ab_3 */
    big_sub(&ab_coefs[1], &ab_coefs[1], &ab_coefs[3]);

    // printf("step (ab)_1 = ");
    // big_print(&ab_coefs[1]);

    // for (int i = 0; i < 5; i++) {
    //     printf("ab_%d = ", i);
    //     big_print(&ab_coefs[i]);
    // }

    /* Now, the result is ab_coefs evaluated at 2^64^len_segment. */
    bigint base;
    big_init(&base);
    big_two_to_pwr(&base, 64 * len_segment);

    bigint reversed_ab_coefs[5]; 
    for (int i = 0; i < 5; i++) {
        reversed_ab_coefs[i] = ab_coefs[4 - i];
    }

    for (int i = 0; i < 5; i++) {
        // printf("rab_%d = ", i);
        // big_print(&reversed_ab_coefs[i]);
    }

    big_eval_polynomial(X, reversed_ab_coefs, 5, &base);
    X->signum = A->signum * B->signum;
    
    /* Tear down all the local variables :( */
    big_free(&a_i);
    big_free(&b_i);
    big_free(&TWO);
    big_free(&THREE);
    big_free(&FOUR);
    big_free(&base);

    for (int i = 0; i < 5; i++) {
        big_free(&reversed_ab_coefs[i]);
        big_free(&ab_pts[i]);
    }
    return 0; 
} 


/* Gets the `bit_num` most significant digit (indexed from 0) from the array */
uint8_t get_bit_from_array(big_uint *data, size_t num_limbs, size_t bit_num) {
    if (bit_num >= 64 * num_limbs) {
        return 0; 
    }
    size_t limb = num_limbs - (bit_num / 64) - 1; 
    size_t idx = bit_num % 64;
    assert(limb >= 0);
    return (data[limb] >> idx) & 0x01; 
}

/* Sets the `bit_num` most significant digit (indexed from 0) to `val` */
void set_bit_in_array(big_uint *data, size_t num_limbs, size_t bit_num, uint8_t val) {
    size_t limb = num_limbs - (bit_num / 64) - 1; 
    size_t idx = bit_num % 64;
    if (val == 1) {
        data[limb] |= (1UL << idx);
    } else if (val == 0){
        data[limb] &= ~(1UL << idx);
    } else {
        assert(false && "Can't set bit t something other than 0 or 1");
    }
}

int big_div(bigint *Q, bigint *R, const bigint *A, const bigint *B) {

    if (Q == NULL && R == NULL){
        return 0;
    }
    if (B->num_limbs == 0){
        return ERR_BIGINT_DIVISION_BY_ZERO;
    }

    bigint abs_A, abs_B;
    big_init(&abs_A);
    big_copy(&abs_A, A);
    abs_A.signum = 1;

    big_init(&abs_B);
    big_copy(&abs_B, B);
    abs_B.signum = 1;

    // If denominator is greater, quotient is 0 and remainder is A
    if (big_cmp(&abs_A, &abs_B) < 0) {
        if (Q != NULL) {
            big_copy(Q, &BIG_ZERO);
            if (A->signum != B->signum && A->num_limbs > 0) {
                bigint one;
                big_init(&one);
                big_set_nonzero(&one, 1);
                big_sub(Q, Q, &one);
            }
        }
        if (R != NULL) {
            big_copy(R, A);
            if (R->signum == -1) {
                big_add(R, R, &abs_B);
            }
        }
        big_free(&abs_B);
        big_free(&abs_A);
        return 0;
    }

    big_free(&abs_B);
    size_t len_A = big_bitlen(A);
    size_t len_B = big_bitlen(B);

    if (len_B == 1) {
        // Q = A
        // R = 0 
        if (R != NULL) {
            big_copy(R, &BIG_ZERO);
        }

        if (Q != NULL) {
            big_copy(Q, A);
            Q->signum = A->signum * B->signum;
        }
        big_free(&abs_A);
        return 0;
    }

    // q has n - t + 1 bits
    // = (n + 1) - (t + 1) + 1 bits
    // where len_A == n + 1, len_B = t + 1
    size_t len_Q_bits = len_A - len_B + 1;
    size_t Q_limbs = len_Q_bits / 64;

    if ((len_A - len_B + 1) % 64 != 0) {
        Q_limbs++;
    }

    big_uint *Q_data = calloc(Q_limbs, sizeof(big_uint));
    
    if (Q_data == NULL) {
        return ERR_BIGINT_ALLOC_FAILED;
    }

    bigint TWO;
    big_init(&TWO);
    big_set_nonzero(&TWO, 2);

    // Step 2
    bigint B_NT; // b^(n-t)
    big_init(&B_NT);
    big_set_nonzero(&B_NT, 1);

    // n - t = (n + 1) - (t + 1) = len_A - len_B
    for (size_t i = 0; i < len_A - len_B; i++) {
        big_mul(&B_NT, &B_NT, &TWO);
    }

    bigint YBNT; // y * b^(n-t)
    big_init(&YBNT);
    big_mul(&YBNT, B, &B_NT);
    YBNT.signum = 1;

    // This was a while loop before. Text afterwards says it should
    // only happen once for B >= floor(b/2) = 1, which we assume to be
    // true. 
    int count = 0;
    while (big_cmp(&abs_A, &YBNT) != -1) {
        // Converting n - t to an index in Q:
        // n - t % 64 is how many bits it takes up in Q_data[0]
        // Take 64 minus this to index from the left side of Q_data
        //printf("Step 2 happens\n");
        set_bit_in_array(Q_data, Q_limbs, len_Q_bits - 1, 1); 
        big_sub(&abs_A, &abs_A, &YBNT);
        count++;
    }

    for (size_t i = len_A - 1; abs_A.num_limbs != 0 && i >= len_B; i--) {
        // check boundary conditions:
        // this should start at the most significant digit of A
        //    and end at the (t+1)th most significant digit. 
        // the starting digit is index 64 - (A_len % 64) - 1 in A[0]
        // then increases upwards

        if (LOG_DEBUG) {
            printf("i = %zu\n", i);
        }

        uint8_t x_i = get_bit_from_array(abs_A.data, abs_A.num_limbs, i);
        uint8_t x_i_minus_1 = get_bit_from_array(abs_A.data, abs_A.num_limbs, i - 1);
        uint8_t x_i_minus_2 = get_bit_from_array(abs_A.data, abs_A.num_limbs, i - 2); 

        if (LOG_DEBUG) {
            // printf("x_i = %d, x_i_minus_1 = %d, x_i_minus_2 = %d\n", x_i, x_i_minus_1, x_i_minus_2);
        }
        
        // otherwise, t would be less, so this is fixed
        uint8_t y_t = 0x01; 

        size_t i_minus_t_minus_1 = i - len_B;

        if (LOG_DEBUG) {
            // printf("i_minus_t_minus_1 = %zu\n", i_minus_t_minus_1);
        }

        // printf("X->data[0] = %lu\n", abs_A.data[0]);

        /*if (i_minus_t_minus_1 == 49) {
            printf("Josh Hejna\n");
        } */

        if (x_i == y_t) {
            if (LOG_DEBUG) {
                // printf("x_i == y_t\n");
            }
            set_bit_in_array(Q_data, Q_limbs, i_minus_t_minus_1, 1);
        } else {
            if (LOG_DEBUG) {
                // printf("x_i != y_t\n");
            }
            if ((x_i * 2 + x_i_minus_1) % 2 == 1) {
                if (LOG_DEBUG) {
                    // printf("x_i != y_t\n");
                }
                set_bit_in_array(Q_data, Q_limbs, i_minus_t_minus_1, 1);
            } 
        }


        // 3.2
        uint8_t LHS = get_bit_from_array(Q_data, Q_limbs, i_minus_t_minus_1);
        LHS *= (2 + get_bit_from_array(B->data, B->num_limbs, len_B - 2));

        uint8_t RHS = 4 * x_i + 2 * x_i_minus_1 + x_i_minus_2;

        if (LHS > RHS) {
            set_bit_in_array(Q_data, Q_limbs, i_minus_t_minus_1, 0);
        }

        if (LOG_DEBUG) {
            // printf("%d %d\n", LHS, RHS);
        }

        // 3.3
        if (get_bit_from_array(Q_data, Q_limbs, i_minus_t_minus_1) != 0){
            if (LOG_DEBUG) {
                // printf("3.3 case\n");
            }
            bigint YBIT1;
            big_init(&YBIT1);
            big_set_nonzero(&YBIT1, 1);
            for (size_t j = 0; j < i_minus_t_minus_1; j++) {
                big_mul(&YBIT1, &YBIT1, &TWO);
            }
            big_mul(&YBIT1, &YBIT1, B);
            YBIT1.signum = 1;
            if (LOG_DEBUG) {
                printf("prev X = ");
                big_print(&abs_A);
                printf("Subtracting YBIT1 = ");
                big_print(&YBIT1);
            }
            big_sub(&abs_A, &abs_A, &YBIT1);
            
            if (abs_A.signum == -1) {
                // 3.4, no need to recompute
                if (LOG_DEBUG) {
                    printf("3.4\n");
                }
                big_add(&abs_A, &abs_A, &YBIT1);
                assert(get_bit_from_array(Q_data, Q_limbs, i_minus_t_minus_1) == 1);
                set_bit_in_array(Q_data, Q_limbs, i_minus_t_minus_1, 0); // assume q[i-i-1] == 1
            }
            big_free(&YBIT1);
        } else if (abs_A.signum == -1){
            // 3.4 computing YBIT1 for the first time
            bigint YBIT1;
            big_init(&YBIT1);
            big_set_nonzero(&YBIT1, 1);
            for (size_t j = 0; j < i_minus_t_minus_1; j++) {
                big_mul(&YBIT1, &YBIT1, &TWO);
            }
            big_mul(&YBIT1, &YBIT1, B);
            YBIT1.signum = 1;
            big_add(&abs_A, &abs_A, &YBIT1);
            assert(get_bit_from_array(Q_data, Q_limbs, i_minus_t_minus_1) == 1);
            set_bit_in_array(Q_data, Q_limbs, i_minus_t_minus_1, 0); // assume q[i-i-1] == 1
            big_free(&YBIT1);
        }
        
        // printf("X = ");
        // big_print(&abs_A);

        // printf("q_i-t-1 = %d\n", get_bit_from_array(Q_data, Q_limbs, i_minus_t_minus_1));
    }

    if (R != NULL) {
        big_copy(R, &abs_A);
        if (R->signum == -1) {
            big_add(R, R, &abs_B);
        }
        trim_limbs(R);
    }
    if (Q != NULL) {
        if (Q->data != NULL) {
            free(Q->data);
        }
        Q->data = Q_data;
        Q->num_limbs = Q_limbs;
        Q->signum = A->signum * B->signum;
        trim_limbs(Q);

        if (A->signum != B->signum && big_cmp(&abs_A, &BIG_ZERO) != 0) {
            bigint one;
            big_init(&one);
            big_set_nonzero(&one, 1);
            big_sub(Q, Q, &one);

            if (R != NULL && A->signum == -1 && B->signum != -1) {
                // R = B - R
                big_sub(R, B, R);
            }
        }
    }

    big_free(&abs_A);
    big_free(&TWO);
    big_free(&B_NT);
    big_free(&YBNT);
    return 0; 
}

int big_mod(bigint *R, const bigint *A, const bigint *B) {
    bigint Q;
    big_init(&Q);
    big_div(&Q, R, A, B);
    big_free(&Q);
    return 0;
}

int big_gcd(bigint *G, const bigint *A, const bigint *B) {
    bigint x;
    big_init(&x);
    bigint y;
    big_init(&y);
    if (big_cmp(A, B) >= 0) {
        big_copy(&x, A);
        big_copy(&y, B); 
    }
    else {
        big_copy(&x, B);
        big_copy(&y, A);
    }
    bigint BASE;
    BASE.signum = 1;
    BASE.num_limbs = 2;
    BASE.data = malloc(sizeof(big_uint) * 2);
    BASE.data[0] = 1;
    BASE.data[1] = 0;

    // int count = 0;

    while (big_cmp(&y, &BASE) == 1 || big_cmp(&y, &BASE) == 0) {
        // 1.1   
        big_sdbl tilde_x = x.data[0];
        big_sdbl tilde_y = y.data[0];
        
        if (x.num_limbs > y.num_limbs) {
            tilde_y = 0;
        }

        big_sdbl AA = 1, BB = 0, C = 0, D = 1, t;

        while (tilde_y + C != 0 && tilde_y + D != 0) {
            big_sdbl q = (tilde_x + AA) / (tilde_y + C);
            big_sdbl q_prime = (tilde_x + BB) / (tilde_y + D);

            if (q != q_prime) {
                break;
            }

            t = AA - q * C;
            AA = C;
            C = t;
            t = BB - q * D;
            BB = D;
            D = t;
            t = tilde_x - q * tilde_y;
            tilde_x = tilde_y;
            tilde_y = t;
        }

        // 1.4
        if (BB == 0) {
            bigint T;
            big_init(&T);
            big_mod(&T, &x, &y);
            big_copy(&x, &y);
            big_copy(&y, &T);
            big_free(&T);
        } else {
            // T ← Ax + By
            bigint T;
            big_init(&T);
            bigint ax;
            big_init(&ax);
            if (AA < 0) {
                big_set_nonzero(&ax, -AA);
                ax.signum = -1;
            } else if (AA == 0) {
                ax = BIG_ZERO;
            } else {
                big_set_nonzero(&ax, AA);
            }
            big_mul(&ax, &ax, &x);

            bigint by;
            big_init(&by);
            if (BB < 0) {
                big_set_nonzero(&by, -BB);
                by.signum = -1;
            } else if (BB == 0) {
                by = BIG_ZERO;
            } else {
                big_set_nonzero(&by, BB);
            }
            big_mul(&by, &by, &y);

            big_add(&T, &ax, &by);
        

            // u ← Cx + Dy
            bigint U;
            big_init(&U);
            bigint cx;
            big_init(&cx);
            if (C < 0) {
                big_set_nonzero(&cx, -C);
                cx.signum = -1;
            } else if (C == 0) {
                cx = BIG_ZERO;
            } else {
                big_set_nonzero(&cx, C);
            }
            big_mul(&cx, &cx, &x);

            bigint dy;
            big_init(&dy);
            if (D < 0) {
                big_set_nonzero(&dy, -D);
                dy.signum = -1;
            } else if (D == 0) {
                dy = BIG_ZERO;
            } else {
                big_set_nonzero(&dy, D);
            }
            big_mul(&dy, &dy, &y);

            big_add(&U, &cx, &dy);

            big_copy(&x, &T);
            big_copy(&y, &U);

            big_free(&T);
            big_free(&U);

            big_free(&ax);
            big_free(&by);
            big_free(&cx);
            big_free(&dy);

            trim_limbs(&x);
            trim_limbs(&y);
        }
    }

    // 2.1
    bigint *zero = malloc(sizeof(bigint));
    *zero = BIG_ZERO;
    while (big_cmp(&y, zero) != 0) {
        bigint *r = malloc(sizeof(bigint));
        big_init(r);
        big_mod(r, &x, &y);
        big_copy(&x, &y);
        big_copy(&y, r);
        big_free(r);
        free(r);
    }
    
    // 2.2
    big_copy(G, &x);
    
    big_free(&x);
    big_free(&y);
    big_free(zero);
    big_free(&BASE);
    free(zero);

    return 0;
}

// int big_inv_mod(bigint *X, const bigint *A, const bigint *N) {
//     // 2.107
//     bigint one;
//     big_init(&one);
//     big_set_nonzero(&one, 1);

//     // no Step 1 by these conditions
//     if (big_cmp(N, &one) != 1){
//         big_free(&one);
//         return ERR_BIGINT_BAD_INPUT_DATA;
//     }
//     if (big_cmp(A, &BIG_ZERO) == 0){
//         return ERR_BIGINT_NOT_ACCEPTABLE;
//     }

//     bigint temp;
//     big_init(&temp);
//     big_gcd(&temp, A, N);
//     if (big_cmp(&temp, &one) != 0){
//         // A only has inverse mod N if gcd (A, N) == 1
//         big_free(&one);
//         big_free(&temp);
//         return ERR_BIGINT_NOT_ACCEPTABLE;
//     }  

//     // Determine which is bigger, A or N
//     bigint *big = malloc(sizeof(bigint));
//     big_init(big);
//     bigint *smol = malloc(sizeof(bigint));
//     big_init(smol);
//     bool A_is_small = false;
//     if (big_cmp(A, N) != -1) {
//         big_copy(big, A);
//         big_copy(smol, N); 
//     } else {
//         big_copy(big, N);
//         big_copy(smol, A);
//         A_is_small = true;
//     }

//     bigint x1;
//     bigint x2;
//     bigint y1;
//     bigint y2;
//     big_init(&x1);
//     big_init(&x2);
//     big_init(&y1);
//     big_init(&y2);
//     big_set_nonzero(&x1, 0);
//     big_set_nonzero(&x2, 1);
//     big_set_nonzero(&y1, 1);
//     big_set_nonzero(&y2, 0);

//     bigint q;
//     bigint r;
//     bigint x;
//     bigint y;
//     big_init(&q);
//     big_init(&r);
//     big_init(&x);
//     big_init(&y);

//     // smol = b and big = a in the 2.107 algo

//     if (big_cmp(smol, &BIG_ZERO) == 0) {
//         big_set_nonzero(X, 1);
//     }


//     while (big_cmp(smol, &one) != -1) {  // effectively: while smol > 0
//         // printf("LOOP\n");
//         // printf("q = ");
//         // big_print(&q);
//         // printf("r = ");
//         // big_print(&r);
//         // printf("x = ");
//         // big_print(&x);
//         // printf("y = ");
//         // big_print(&y);
//         // printf("a = ");
//         // big_print(big);
//         // printf("b = ");
//         // big_print(smol);
//         // printf("x1 = ");
//         // big_print(&x1);
//         // printf("x2 = ");
//         // big_print(&x2);
//         // printf("y1 = ");
//         // big_print(&y1);
//         // printf("y2 = ");
//         // big_print(&y2);


//         big_div(&q, NULL, big, smol);
//         big_mul(&r, &q, smol);
//         big_sub(&r, big, &r);
//         big_mul(&x, &q, &x1);
//         big_sub(&x, &x2, &x);
//         big_mul(&y, &q, &y1);
//         big_sub(&y, &y2, &y);

//         big_copy(big, smol);
//         big_copy(smol, &r);
//         big_copy(&x2, &x1);
//         big_copy(&x1, &x);
//         big_copy(&y2, &y1);
//         big_copy(&y1, &y);

//         // printf("ENDLOOP\n");
//     }
//     printf("b = ");
//     big_print(smol);

//     printf("d = ");
//     big_print(big);

//     printf("x2 = ");
//     big_print(&x2);

//     printf("y2 = ");
//     big_print(&y2);

//     big_copy(X, &x2);

//     if (A_is_small) {
//         printf("A_is_small\n");
//         big_copy(X, &y2);
//     }
//     // make sure big = 1, otherwise, no modular inverse


//     big_copy(X, &x2);

//     big_free(big);
//     big_free(smol);
//     big_free(&one);
//     big_free(&temp);
//     big_free(&x1);
//     big_free(&x2);
//     big_free(&y1);
//     big_free(&y2);
//     big_free(&q);
//     big_free(&r);
//     big_free(&x);
//     big_free(&y);

//     free(big);
//     free(smol);
//     return 0;
// }

int big_inv_mod(bigint *X, const bigint *A, const bigint *N) {
    // 2.107
    bigint one;
    big_init(&one);
    big_set_nonzero(&one, 1);

    // no Step 1 by these conditions
    if (big_cmp(N, &one) != 1){
        big_free(&one);
        return ERR_BIGINT_BAD_INPUT_DATA;
    }
    if (big_cmp(A, &BIG_ZERO) == 0){
        return ERR_BIGINT_NOT_ACCEPTABLE;
    }

    bigint temp;
    big_init(&temp);
    big_gcd(&temp, A, N);
    if (big_cmp(&temp, &one) != 0){
        // A only has inverse mod N if gcd (A, N) == 1
        big_free(&one);
        big_free(&temp);
        return ERR_BIGINT_NOT_ACCEPTABLE;
    }  

    // Determine which is bigger, A or N
    bigint *big = malloc(sizeof(bigint));
    big_init(big);
    bigint *smol = malloc(sizeof(bigint));
    big_init(smol);
    bool A_is_small = false;
    if (big_cmp(A, N) != -1) {
        big_copy(big, A);
        big_copy(smol, N); 
    } else {
        big_copy(big, N);
        big_copy(smol, A);
        A_is_small = true;
    }

    bigint x1;
    bigint x2;
    bigint y1;
    bigint y2;
    big_init(&x1);
    big_init(&x2);
    big_init(&y1);
    big_init(&y2);
    big_set_nonzero(&x1, 0);
    big_set_nonzero(&x2, 1);
    big_set_nonzero(&y1, 1);
    big_set_nonzero(&y2, 0);

    bigint q;
    bigint r;
    bigint x;
    bigint y;
    big_init(&q);
    big_init(&r);
    big_init(&x);
    big_init(&y);

    // smol = b and big = a in the 2.107 algo

    if (big_cmp(smol, &BIG_ZERO) == 0) {
        big_set_nonzero(X, 1);
    }

    int loop = 0;
    while (big_cmp(smol, &one) != -1) {  // effectively: while smol > 0
        // printf("LOOP: %d\n", );
        // printf("q = ");
        // big_print(&q);
        // printf("r = ");
        // big_print(&r);
        // printf("x = ");
        // big_print(&x);
        // printf("y = ");
        // big_print(&y);
        // printf("a = ");
        // big_print(big);
        // printf("b = ");
        // big_print(smol);
        // printf("x1 = ");
        // big_print(&x1);
        // printf("x2 = ");
        // big_print(&x2);
        // printf("y1 = ");
        // big_print(&y1);
        // printf("y2 = ");
        // big_print(&y2);


        big_div(&q, NULL, big, smol);
        big_mul(&r, &q, smol);
        big_sub(&r, big, &r);

        big_mul(&x, &q, &x1);
        big_sub(&x, &x2, &x);
        
        big_mul(&y, &q, &y1);
        big_sub(&y, &y2, &y);

        big_copy(big, smol);
        big_copy(smol, &r);
        big_copy(&x2, &x1);
        big_copy(&x1, &x);
        big_copy(&y2, &y1);
        big_copy(&y1, &y);

        // printf("ENDLOOP\n");
    }
    printf("b = ");
    big_print(smol);

    printf("d = ");
    big_print(big);

    printf("x2 = ");
    big_print(&x2);

    printf("y2 = ");
    big_print(&y2);

    big_copy(X, &x2);

    if (A_is_small) {
        printf("A_is_small\n");
        big_copy(X, &y2);
    }

    while (X->signum == -1) {
        // printf("new X = ");
        // big_print(X);
        if (A_is_small){
            big_add(X, X, A);
        } else {
            big_add(X, X, N);
        }
    }
    // make sure big = 1, otherwise, no modular inverse


    big_free(big);
    big_free(smol);
    big_free(&one);
    big_free(&temp);
    big_free(&x1);
    big_free(&x2);
    big_free(&y1);
    big_free(&y2);
    big_free(&q);
    big_free(&r);
    big_free(&x);
    big_free(&y);

    free(big);
    free(smol);
    return 0;
}


int mont_mul(bigint *result, bigint *m, bigint *x, bigint *y) {
    // algo 14.36, not tested
    // Base 2^64
    bigint b;
    b.signum = 1;
    b.num_limbs = 2;
    b.data = malloc(sizeof(big_uint) * 2);
    b.data[0] = 1;
    b.data[1] = 0;

    bigint m_prime;
    big_init(&m_prime);
    m_prime.signum *= -1;
    big_inv_mod(&m_prime, m, &b);

    bigint y_0;
    big_init(&y_0);
    if (y->num_limbs > 0) {
        big_set_nonzero(&y_0, y->data[y->num_limbs - 1]);
    } else {
        y_0 = BIG_ZERO;
    }

    bigint A;
    big_init(&A);
    bigint x_i;
    big_init(&x_i);
    bigint xiy0;
    big_init(&xiy0);
    bigint a0;
    big_init(&a0);
    bigint axy;
    big_init(&axy);
    bigint axym_prime;
    big_init(&axym_prime);
    bigint u_i;
    big_init(&u_i);
    bigint xiy;
    big_init(&xiy);
    bigint uim;
    big_init(&uim);
    bigint Axiy;
    big_init(&Axiy);
    bigint Axiyuim;
    big_init(&Axiyuim);
    bigint remainder;
    big_init(&remainder);
    
    for (size_t i = 0; i < x->num_limbs; i++) {
        // 2.1 
        if (x->data[x->num_limbs - i - 1] != 0){
            big_set_nonzero(&x_i, x->data[x->num_limbs - i - 1]);
            big_mul(&xiy0, &x_i, &y_0);
        } else {
            big_copy(&xiy0, &BIG_ZERO);
        }
        if (A.num_limbs > 0){
            big_set_nonzero(&a0, A.data[A.num_limbs - 1]);
            big_add(&axy, &a0, &xiy0);
        }
        big_mul(&axym_prime, &axy, &m_prime);
        big_mod(&u_i, &axym_prime, &b);

        // 2.2
        big_mul(&xiy, &x_i, y);
        big_mul(&uim, &u_i, m);
        big_add(&Axiy, &A, &xiy);
        big_add(&Axiyuim, &Axiy, &uim);
        
        // ignore remainder
        big_div(&A, &remainder, &Axiyuim, &b);
    }
        
    // 3
    if (big_cmp(&A, m) >= 0) {
        big_sub(&A, &A, m);
    }
    
    // 4
    big_copy(result, &A);

    big_free(&x_i);
    big_free(&xiy0);
    big_free(&a0);
    big_free(&axy);
    big_free(&m_prime);
    big_free(&axym_prime);
    big_free(&u_i);
    big_free(&xiy);
    big_free(&uim);
    big_free(&Axiy);
    big_free(&Axiyuim);
    big_free(&remainder);
    big_free(&b);
    big_free(&A);
    big_free(&y_0);
    return 0;
}

uint8_t get_bit_from_arr(big_uint *data, int idx, size_t data_idx, size_t sigfigs){
    // from left to right, 0 -> least sig bit
    // not tested
    int max_idx = data_idx == 0 ? sigfigs-1 : 15;
    size_t loc = (max_idx - (idx % 16));
    // printf("loc = %d\n", loc);
    // printf("data_idx = %d\n", data_idx);
    // printf("ret = %d\n", (data[data_idx] >> loc) & 1);
    // printf("----------------------\n");
    return (data[data_idx] >> loc) & 1;
}

int big_exp_mod(bigint *X, const bigint *A, const bigint *E, const bigint *N, bigint *_RR) {
    // algo 14.94
    // 1
    bigint R;
    big_init(&R);
    big_two_to_pwr(&R, 64 * N->num_limbs);
    
    bigint R2;
    big_init(&R2);
    big_two_to_pwr(&R2, 2 * 64 * N->num_limbs);
    
    bigint R2_mod_N;
    big_init(&R2_mod_N);
    big_mod(&R2_mod_N, &R2, N);
    big_copy(_RR, &R2_mod_N);
    
    bigint x_tilde;
    big_init(&x_tilde);
    mont_mul(&x_tilde, N, A, &R2_mod_N);
    // printf("x_tilde = ");
    
    bigint R_mod_N;
    big_init(&R_mod_N);
    big_mod(&R_mod_N, &R, N);
    
    big_copy(A, &R_mod_N);
 
    // 2
    bigint mont_A;
    big_init(&mont_A);

    size_t e_bits = (E->num_limbs-1) * 16;
    size_t sigfigs = 0;
    uint64_t first_idx = E->data[0];
    while (first_idx){
        e_bits++;
        sigfigs++;
        first_idx >>= 1;
    }
    // printf("e_bits = %d\n", e_bits);
    // printf("sigfigs = %d\n", sigfigs);
    
    for (size_t i = 0; i < e_bits; i++) {
        // 2.1
        mont_mul(&mont_A, N, A, A);
        big_copy(A, &mont_A);

        // 2.2
        int d_idx = (e_bits - i)/16;
        uint8_t curr_e_bit = get_bit_from_arr(E->data, i, d_idx, sigfigs);
        if (curr_e_bit == 1) {
            mont_mul(&mont_A, N, A, &x_tilde);
            big_copy(A, &mont_A); // can condense into 1 line
        }
    }

    // 3
    bigint one;
    big_init(&one);
    big_set_nonzero(&one, 1);
    mont_mul(X, N, A, &one);

    big_free(&R);
    big_free(&R2);
    big_free(&R2_mod_N);
    big_free(&x_tilde);
    big_free(&R_mod_N);
    big_free(&mont_A);
    big_free(&one);

    return 0;
}

bigint *load_primes(const char *filename, size_t num_primes) {
    bigint *result = malloc(num_primes * sizeof(bigint));
    
    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char *line = NULL;
    size_t len = 0;

    ssize_t read; 

    for (size_t i = 0; i < num_primes; i++) {
        big_init(&result[i]);
    }

    int count = 0; 
    while ((read = getline(&line, &len, f)) != -1 && count < (int)num_primes) {
        line[read - 1] = '\0';
        big_read_string(&result[count], (const char *)line);

        count++;
    }

    free(line);
    return result; 
}

int big_is_prime(const bigint *X) {
    bigint ONE, TWO, FOUR;
    big_init(&ONE);
    big_set_nonzero(&ONE, 1);
    big_init(&TWO);
    big_set_nonzero(&TWO, 2);
    big_init(&FOUR);
    big_set_nonzero(&FOUR, 4);

    // corner cases
    if (big_cmp(X, &ONE) <= 0 || big_cmp(X, &FOUR) == 0) {
        big_free(&ONE);
        big_free(&TWO);
        big_free(&FOUR);
        return ERR_BIGINT_NOT_ACCEPTABLE;
    }
    if (big_cmp(X, &FOUR) < 0) {
        // 2 or 3
        big_free(&ONE);
        big_free(&TWO);
        big_free(&FOUR);
        return 0;
    }
    
    bigint remainder_mod_2; 
    big_init(&remainder_mod_2);
    big_mod(&remainder_mod_2, X, &TWO);

    // If X is even, don't even do the rest
    if (big_cmp(&remainder_mod_2, &BIG_ZERO) == 1) {
        big_free(&ONE);
        big_free(&TWO);
        big_free(&FOUR);
        big_free(&remainder_mod_2);
        return ERR_BIGINT_NOT_ACCEPTABLE;
    }

    // Maximize r such that d is odd, x = 2^r * d + 1 
    bigint d, n_minus_one;
    big_init(&d);
    big_init(&n_minus_one);

    // residual = 2^r + d
    big_sub(&n_minus_one, X, &ONE);
    big_copy(&d, &n_minus_one);

    big_uint r = 0;
    // loop to set residual = d
    big_mod(&remainder_mod_2, &d, &TWO);

    while (big_cmp(&remainder_mod_2, &BIG_ZERO) == 0) { // while remainder mod 2 is even, i.e. we can divide out 2
        // residual /= 2
        big_div(&d, &remainder_mod_2, &d, &TWO);
        r++;
    }
    assert (big_cmp(&remainder_mod_2, &BIG_ZERO) == 0);
    size_t NUM_PRIMES = 1229;

    // So, now we should have X = d * 2^r + 1. Can test this if it doesn't work.
    bigint *primes = load_primes("list_of_primes.txt", NUM_PRIMES);

    for (size_t i = 0; i < NUM_PRIMES; i++) {
        // t <- a^d mod n 
        bigint t;
        big_init(&t);
        big_exp_mod(&t, &primes[i], &d, X, NULL);
        
        // if t == 1 or t == n - 1, continue. 
        if (big_cmp(&t, &ONE) == 0 || big_cmp(&t, &n_minus_one)) {
            continue;
        }

        // repeat r - 1 times:
        bool next_iter = false;
        for (size_t j = 0; j < r - 1; j++) {
            // x <- x^2 mod n
            big_exp_mod(&t, &t, &TWO, X, NULL);

            // if x = n - 1, then continue. 
            if (big_cmp(&t, &n_minus_one) == 0) {
                next_iter = true;
                break;
            }
        }

        if (!next_iter) {
            return ERR_BIGINT_NOT_ACCEPTABLE; // composite
        }
    }

    for (size_t i = 0; i < NUM_PRIMES; i++) {
        big_free(&primes[i]);
    }
    free(primes);

    return 0;
}

big_uint random_limb(size_t nbits) {
    big_uint mask = ~(0xFFFFFFFFFFFFFFFF << (64 - nbits)); 
    uint16_t a = rand();
    uint16_t b = rand();
    uint16_t c = rand();
    uint16_t d = rand();
    big_uint full_rand = 0;
    full_rand |= a; 
    full_rand |= ((uint64_t)b << 16);
    full_rand |= ((uint64_t)c << 32);
    full_rand |= ((uint64_t)d << 48);
    full_rand &= mask;
    return full_rand;
}

int random_bigint(bigint *X, size_t nbits) {
    size_t n_limbs = nbits / 64;
    if (nbits % 64 != 0) {
        n_limbs++;
    }
    resize_limbs(X, n_limbs);
    X->data[0] = random_limb(nbits % 64);
    for (size_t i = 1; i < X->num_limbs; i++) {
        X->data[i] = random_limb(64);
    }
    return 0;
}

int big_gen_prime(bigint *X, size_t nbits) {
    // Process for this procedure:
    // 1. Preselect a random number with n-bits
    // 2. Ensure the chosen number is not divisible by the first few hundred primes 
    //    (these are pre-generated)
    random_bigint(X, nbits); 
    while (big_is_prime(X) == 1) {
        random_bigint(X, nbits); 
    }
    return 0;
}

int big_two_to_pwr(bigint *X, size_t pwr) {
    size_t n_limbs_for_exp = 1 + pwr / 64;
    if (pwr % 64 != 0) {
        n_limbs_for_exp++;
    }

    if (X->data != NULL) {
        free(X->data);
    }

    X->data = calloc(n_limbs_for_exp, sizeof(big_uint));
    X->num_limbs = n_limbs_for_exp;
    X->signum = 1;
    X->data[0] = (1UL << (pwr % 64));
    return 0;
}
