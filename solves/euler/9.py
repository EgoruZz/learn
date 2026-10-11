# x = m^2 - n^2, y = 2*m*n, z = m^2 + n^2
# x > 0: 0 < n < m - ПО НУЖДЕ
# k * (x, y, z), k == 1: gcd(n,m)=1 и n!=m(mod2) - ПО НУЖДЕ

# решение: x+y+z=1000 <==> m^2+mn-500=0 <==> n = 500/m - m
m = 20
n = 5
x = m*m - n*n
y = 2*m*n
z = m*m + n*n
print(x*x + y*y == z*z)
print(x*y*z)