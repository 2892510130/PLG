#include <iostream>

int main()
{
    int val = 5;
    auto l = [val]() mutable {std::cout << "Hello, it is " << ++val << '\n';};

    l();
    std::cout << val << '\n';
}