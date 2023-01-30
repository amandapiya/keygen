import random
import math
import sympy


for a in range(2, 10000):
    if sympy.isprime(a):
        print("{:x}".format(a))
        
    '''
    if aneg * a == bneg * b:
        ans = 0
    elif aneg * a > bneg * b:
        ans = 1
    else:
        ans = -1

    print("{:x},{:x},{:x}".format(aneg * a, bneg * b, ans))
    '''

