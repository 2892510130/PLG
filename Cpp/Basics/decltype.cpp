#include <iostream>
#include <vector>

using namespace std;

using size_t = decltype(sizeof(0));//sizeof(a)的返回值为size_t类型
using ptrdiff_t = decltype((int*)0 - (int*)0);
using nullptr_type = decltype(nullptr);
vector<int >vec;
typedef decltype(vec.begin()) vectype;

struct 
{
    int d ;
    double b;
} anon_s;

int main()
{
    anon_s.d = 5;
    // anon_s a;  // error
    decltype(anon_s) a;
    a.b = 5.5;

    int i = 4;
    int arr[5] = { 0 };
    int *ptr = arr;
    struct S{ double d; }s ;
    void Overloaded(int);
    void Overloaded(char);//重载的函数
    int && RvalRef();
    const bool Func(int);

    //规则一：推导为其类型
    decltype (i) var0; //int 标记符表达式

    decltype (arr) var1; //int[5] 标记符表达式

    decltype (ptr) var2;//int *  标记符表达式

    decltype(s.d) var3;//doubel 成员访问表达式

    //decltype(Overloaded) var4;//重载函数。编译错误。

    //规则二：将亡值。推导为类型的右值引用。

    decltype (RvalRef()) var5 = 1;

    //规则三：左值，推导为类型的引用。

    decltype ((i))var6 = i;     //int&

    decltype (true ? i : i) var7 = i; //int&  条件表达式返回左值。

    decltype (++i) var8 = i; //int&  ++i返回i的左值。

    cout << i << '\n';

    decltype(arr[5]) var9 = i;//int&. []操作返回左值

    decltype(*ptr)var10 = i;//int& *操作返回左值

    decltype("hello")var11 = "hello"; //const char(&)[9]  字符串字面常量为左值，且为const左值。


    //规则四：以上都不是，则推导为本类型

    decltype(1) var12;//const int

    decltype(Func(1)) var13=true;//const bool

    decltype(i++) var14 = i;//int i++返回右值
}