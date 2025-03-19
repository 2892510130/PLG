#include <iostream>

using namespace std;

class A
{
    int a;
    char b;
    long c;
    short d;
};

class B
{
    int a;
    char b;
    long c;
    short d;
    virtual void fun() {};
};

int main()
{   
    cout<<sizeof(int)<<endl;
    cout<<sizeof(char)<<endl;
    cout<<sizeof(long)<<endl;
    cout<<sizeof(short)<<endl;
    cout<<sizeof(A)<<endl; // align to 4, so it is 4 * 4 = 16
    cout<<sizeof(B)<<endl; // 16 + 8, the virtual pointer vptr size is 8 in 64 bit computer
}
