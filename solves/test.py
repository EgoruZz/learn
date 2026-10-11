a = 0
b = 0
for i in range(1, 100+1):
    a += i * i; b += i
b *= b
print(b - a)

c = 0
for i in range(1,101): c += i*i
print(c)