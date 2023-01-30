import random
import math


# a = int(0x11ffffffe00000000010021f23456789fffff0)
# b = int(0xffffffffffffffffffffffe00000000000000)
# print(hex(a), " + ", hex(b), " = ", hex(a+b))
# print(hex(a), " - ", hex(b), " = ", hex(a-b))

# for i in range(30):
#     alen = random.randint(64, 100)
#     blen = random.randint(64, 100)
#     # plen = random.randint(1, 100)
#     a = random.getrandbits(alen) * random.choice([-1, 1])
#     b = random.getrandbits(blen) * random.choice([-1, 1])
#     # p = random.getrandbits(plen) # non neg
#     # a <<= 64
#     print("{:x},{:x},{:x}".format(a, b, a * b))

a = int(0x20000000000000001)
b = int(0x50000000000000001)
# print(hex(a),hex(b),hex(a*b))
print(hex(a*b))
# e0ca6fa1b88349d444d031217e49ea1e1ccfbb15432e1d0e7a73ba7d5bba4d13c870e2313d4d35d206a8f6034cb2e0ca6fa1b88349d444d031217e49ea1e1ccfbb15432e1d0e7a73ba7d5bba4d13c870e2313d4d35d206a8f6034cb2
# e0ca6fa1b88349d444d031217e49ea1e1ccfbb15432e1d0e7a73ba7d5bba4d13c870e2313d4d35d206a8f6034cb2