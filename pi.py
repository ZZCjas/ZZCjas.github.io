import math
import time
import decimal
x=int(input("请输入计算到圆周率的小数点后几位:"))
y=int(input("请输入迭代次数(迭代次数越多,结果越精确):"))
print("计算中...")
D=decimal.Decimal
decimal.getcontext().prec=x
a=1
b=1/D(2).sqrt()
t=1/4
p=1
w=0
time_start=time.time()
while w<y:
    a1=(a+b)/2
    b1=D(a*b).sqrt()
    t1=D(t)-p*((a-(a+b)/2)**2)
    p1=2*p
    a=a1
    b=b1
    t=t1
    p=p1
    w=w+1
    pi=D(((a1+b1)**2)/(4*t1))
time_end=time.time()
print(f'在迭代{w}次后,算得圆周率小数点后{x}位为:\n{(pi)}\n')
print(f'圆周率小数点后{x}位计算完成,花了{time_end-time_start}秒')
