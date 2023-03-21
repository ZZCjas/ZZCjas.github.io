import math
import time
import decimal

ZZC=int(input("请输入计算精度:"))

D = decimal.Decimal
decimal.getcontext().prec = ZZC

a = 1
b = 1/D(2).sqrt()
t = 1/4
p = 1

w = 0

ZZZC=int(input("请输入迭代次数:"))

time_start = time.time()

while w<ZZZC:
    a1 = (a+b)/2
    b1 = D(a*b).sqrt()
    t1 = D(t)-p*((a-(a+b)/2)**2)
    p1 = 2*p

    a = a1
    b = b1
    t = t1
    p = p1

    w = w+1
    pi = D(((a1 + b1) ** 2) / (4 * t1))


time_end = time.time()

print(f'迭代{w}次后,算得圆周率为:\n{(pi)}')
print(f'计算完成,花了{time_end-time_start}秒')
