
def f(x):
    print(x)
    a = 0xffff
    b = 0xffffffffffffffffffffffffffff * (x ** 2) + 0xffffffffffffffffffffffffffffffffffffffffffffffff * x + 0xffffffffffffffffffffffffffffffffffffffffffffffff
    return (a) * (b)

#      0    1       2       3   4
ab = [f(0), f(1), f(-1), f(-2), 0]

print(["{:x}".format(x) for x in ab])

# print(ab)

r = [0, 0, 0, 0, 0]

r[0] = ab[0]

r[4] = ab[4]

r[3] = (ab[3] - ab[1]) // 3
print("{:x}".format(r[3]))

r[1] = (ab[1] - ab[2]) // 2
assert (ab[1] - ab[2]) % 2 == 0
print("{:x}".format(r[1]))

r[2] = ab[2] - ab[0]
print("{:x}".format(r[2]))


assert (r[2] - r[3]) % 2 == 0
r[3] = (r[2] - r[3]) // 2 + 2 * r[4]
print("{:x}".format(r[3]))

r[2] = r[2] + r[1] - r[4]
print("{:x}".format(r[2]))

r[1] = r[1] - r[3]
print("{:x}".format(r[1]))



print(["{:x}".format(x) for x in r])

print("{:x}".format(r[1] * (2 ** 64)))

#-4cdf78fc 751b5946649 fc103128
#-4cdf78fc 9b8b15c3313 fc103128

# 4cdf78fc 9b8b15c3313 fc103128

# -a7ba03e2936275f4d5735ec630400b166eb62ecbd62acc8455589d8c71f5175fe20980362ec0000000000000000
# -b1651b9640300b275f4de69f41f19251760a6c04ee1979acc844c620833891