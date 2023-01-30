import random
import math




BIG_NUM = 999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999
for i in range(50):
    int_num = random.randint(1, BIG_NUM)
    # bits = random.getrandbits(int_num)

    hex_num = str(hex(int_num))[2:]
    num_limbs = math.ceil(len(str(hex_num)) / 16)

    print(f"{hex_num},{num_limbs},{int_num}")
    # print("{:x},1,{}".format(bits, bits))

# Command line: 
# python3 ./tests/generate_write_binary_tests.py > ./test_cases/stress_test_write_binary.csv
