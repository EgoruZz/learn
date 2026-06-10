from math import *

def f(mid, d1, d2, u1, u2):
    free = mid - (mid // d1 + mid // d2) + mid // lcm(d1, d2)
    second = 0
    first = 0
    if (d1 % d2 != 0): second = mid // d1 - mid // lcm(d1, d2)
    if (d2 % d1 != 0): first = mid // d2 - mid // lcm(d1, d2)
    return (first + free >= u1) and (free - max(u1 - first, 0) + second >= u2)

# 3 6 9
# 1 2 4 5 7 8 10 11 13
d1, d2, u1, u2 = map(int, input().split())
ls = 0
rs = int(1e9)

while rs - ls > 1:
    mid = (rs + ls) // 2
    if (f(mid, d1, d2, u1, u2)): rs = mid
    else: ls = mid

print(ls, rs)
print(lcm(2557, 15901))