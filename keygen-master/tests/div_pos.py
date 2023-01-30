import random

for l1 in range(1, 10):
    for l2 in range(1, 10):
        for i in range(5):
            b = 0
            while b == 0:
                alen = random.randint(max(1, (l1-1)), l1)
                blen = random.randint(max(1, (l2-1)), l2)
                a = random.getrandbits(alen)
                b = random.getrandbits(blen)

            aneg = 1
            bneg = 1

            quotient = (a * aneg ) // (b * bneg)

            remainder = (a * aneg) % (b * bneg)

            if remainder < 0:
                remainder += b

            

            print("{:x},{:x},{:x},{:x}".format(aneg * a, bneg * b, quotient, remainder))
            
            '''
            if aneg * a == bneg * b:
                ans = 0
            elif aneg * a > bneg * b:
                ans = 1
            else:
                ans = -1

            print("{:x},{:x},{:x}".format(aneg * a, bneg * b, ans))
            '''

