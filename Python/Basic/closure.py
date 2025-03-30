"""
Closure: in function A we define function B, and B use params of function A.
PS. We need to use nonlocal to make it right.
"""

def lazy_sum(*arg):
    sums = 0
    def sum():
        for i in arg:
            nonlocal sums
            sums = sums + i
        return sums
    return sum

f1 = lazy_sum(1,3,5,7,9)
f2 = lazy_sum(1,3,5,7,9)
print(f1())
print(f2())
print(f1 == f2)
f3 = lazy_sum(1,3,5,7,9)
print(f3())
