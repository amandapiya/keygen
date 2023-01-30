x = 0x33
import math
# x = 9

result = x // 3


# print x as hex
print("{:x}".format(x))

ex = 2
for i in range(8):
    x = x + (x >> ex)
    print(ex)
    print(x)
    print("X = {:x}".format(x))
    ex *= 2

# x = math.ceil(x / 4)
# x = x >> 2
x = x // 4 + 1
print("x = {:x}".format(x))

print("result: {:x}".format(result))
print()
print(x - result)