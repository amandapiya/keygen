import random
import math
# import sympy

def egcd(a, b):
    if a == 0:
        return (b, 0, 1)
    else:
        g, y, x = egcd(b % a, a)
        return (g, x - (b // a) * y, y)

def modinv(a, m):
    g, x, y = egcd(a, m)
    if g == 1:
        print("{:x},{:x},{:x}".format(a, m, x % m))


for i in range(500):
    alen = random.randint(0, 2048)
    # plen = random.randint(1, 100)
    a = random.getrandbits(alen)
    # b = random.randint(1, alen)
    # p = random.getrandbits(plen) # non neg

    print("{:x},{:x}".format(3 * a, a))
# print("{:x},{}".format(b, 0 if sympy.isprime(b) else 1))

'''
if aneg * a == bneg * b:
    ans = 0
elif aneg * a > bneg * b:
    ans = 1
else:
    ans = -1

print("{:x},{:x},{:x}".format(aneg * a, bneg * b, ans))
'''

