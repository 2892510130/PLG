#include <iostream>

using namespace std;

class A
{
public:
    void f() { cout << "A f\n"; }
    void f(int x) { cout << "A f with x\n"; }
    void f(double x) { cout << "A f with x\n"; }
};

class B : public A
{
public:
    using A::f; // will work for all the other f functions!
    void f(int x) { cout << "B f with x\n"; }
};

int main()
{
    A a;
    B b;
    a.f();
    a.f(3);
    a.f(3.1);
    b.f(5);
    b.f();
    b.f(3.1);
}