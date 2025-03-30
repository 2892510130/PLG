#include <iostream>

// typedef int (*func)(int x);

using func = int (*)(int x);

void callback(func f, int k)
{
    int c = f(k);
    std::cout << c << '\n';
}

int test(int x)
{
    return x + 1;
}

int main()
{
    callback(test, 1);
}