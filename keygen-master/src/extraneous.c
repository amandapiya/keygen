// This file holds extraneous code we might want to refer back to



// big_read_string
int new_num_limbs = sizeof(s) / sizeof(s[0]);
    char *result = malloc(new_num_limbs * 4);

    if (X->num_limbs < new_num_limbs) {
        free(X->data);
        X->data = malloc(new_num_limbs * sizeof(big_uint));
    }

    int i = 0;
    while (s[i]) {
        switch (s[i]) {
        case '0':
            result[i * 4] = "0000";
            break;
        case '1':
            result[i * 4] = "0001";
            break;
        case '2':
            result[i * 4] = "0010";
            break;
        case '3':
            result[i * 4] = "0011";
            break;
        case '4':
            result[i * 4] = "0100";
            break;
        case '5':
            result[i * 4] = "0101";
            break;
        case '6':
            result[i * 4] = "0110";
            break;
        case '7':
            result[i * 4] = "0111";
            break;
        case '8':
            result[i * 4] = "1000";
            break;
        case '9':
            result[i * 4] = "1001";
            break;
        case 'A':
        case 'a':
            result[i * 4] = "1010";
            break;
        case 'B':
        case 'b':
            result[i * 4] = "1011";
            break;
        case 'C':
        case 'c':
            result[i * 4] = "1100";
            break;
        case 'D':
        case 'd':
            result[i * 4] = "1101";
            break;
        case 'E':
        case 'e':
            result[i * 4] = "1110";
            break;
        case 'F':
        case 'f':
            result[i * 4] = "1111";
            break;
        default:
            return ERR_BIGINT_INVALID_CHARACTER;
        }
        i++;
    }
    return 0;

