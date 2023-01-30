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


for i in range(50):
    alen = random.randint(0, 100)
    blen = random.randint(0, 100)
    plen = random.randint(1, 100)
    a = random.getrandbits(alen)
    b = random.getrandbits(blen) # non neg, non even
    p = random.getrandbits(plen) # non neg

    if b%2 == 0:
        b |= 1

    a*=random.choice([-1,1])

    print("{:x},{:x},{:x},{:x}".format(a,b,p,pow(a,b,p)))
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

