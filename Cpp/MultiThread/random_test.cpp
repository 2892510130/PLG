#include <iostream>

struct RandomTest
{
    int* a;
    RandomTest* next;
    RandomTest() : a(0), next(0) {}
};


int main()
{
    RandomTest r;
    if (r.a == nullptr) std::cout << "a is nullptr\n";
    if (r.next == nullptr) std::cout << "next is nullptr\n";
}