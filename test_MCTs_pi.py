from random import random
from time import perf_counter
from math import sqrt

points = int(input("Input Point N: "))
cnt = 0

start = perf_counter()
print("Counting...")

for i in range(points):
    x, y = random(), random()
    dis = sqrt(x ** 2 + y ** 2)
    if dis <= 1.0:
        cnt += 1

pi = 4 * cnt / points
print("{:.6f}".format(pi))
print("Time: {:.6f} s".format(perf_counter() - start))
        
