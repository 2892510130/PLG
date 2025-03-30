from functools import cache

def decorator(f):
    def wrapper(*args, **kw):
        print("Before")
        f(*args, **kw)
        print("After, with func name: ", f.__name__)
    return wrapper

@decorator
def fun(str):
    print("In the fun")

@cache
def cache_dec(n:int) -> int:
    """
    Only for functions (one input has one output), so it should not rely on outer var of time.
    """
    if n == 1 or n == 2:
        return n
    else:
        return cache_dec(n - 1) + cache_dec(n - 2)

if __name__ == "__main__":
    fun("Hello World")
    print(cache_dec(4)) # It will cache f(2) when calc f(4) = f(3) + f(2), and use it when calc f(3) = f(2) + f(1)
