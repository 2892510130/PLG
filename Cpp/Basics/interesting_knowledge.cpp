/*
1. & will always come with it's ref obj, except decltype
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;
using std::cin; // this will make only std::cin as cin

constexpr int get_size(); // add inline can avoid multi define problem
int foo();
void cin_example();
void vector_erase_example();
void print_vector(const vector<int> & vec, bool test_arg = true);
void test_fun(int a, int b);
void call_back();
void register_callback(void cb()); // same as void (*cb)()

double gloabl_val = 4.0;
double gloabl_val2 = 6.0;

typedef double *double_ptr;
typedef double &double_ref;
typedef void (*Callback)();

int main()
{
    // 1. dconst &
    double val = 4;
    // int & val_r = val; // This is a error, because compiler tries to cast double to int, which will create a tmp int value
    // This tmp value can not be changed, it is a rvalue, so you can not use changeable & to store it.
    const int & val_r = val;

    // 2. const *
    const double * val_ptr = &val;
    cout << "2. const *: " << val_ptr << endl;
    double val_d = 4.0;
    val_ptr = &val_d; // you can not change *val_ptr
    cout << val_ptr << endl;

    // 3. * const
    double * const c_ptr = &val;
    *c_ptr = 5.0; // you can not change c_ptr
    cout << "3. * const: " << val << endl;
    val = 6;
    cout << *c_ptr << endl;
    // we can even do const * const

    // 4. constexpr *, compare it with 2
    constexpr double * ce_val = &gloabl_val;
    // constexpr double * ce_val = &val; // will cause error because constexpr * must point to a obj with fixed memory, but a val
    // in the function dose not have one, but the global value has fixed memory.
    *ce_val = 5.0;
    // ce_val = &gloabl_val2; // can not do this
    cout << "4. constexpr *: " << gloabl_val << endl;

    // 5. typedef
    double_ptr d = &val; // double_ptr is double *
    double_ref dr = val; // double_ptr is double &
    cout << "5. typedef: " << dr << endl;

    // 6. auto: auto will not add top const (const on the obj, but ref or pointer is not obj), so auto_d is a double
    const double v2 = 4.0;
    auto * auto_p = &v2; // This auto_p is of const double * type, const is saved, because ptr and ref are not obj
    // auto * auto_test = v2; // not correct because compiler can not solve the type
    // auto &h = 42; // wrong
    const auto &h = 42;
    auto auto_d = v2;

    // 7. decltype, only return type, will not excuate the function
    double v3 = 4.5;
    const int &ci = 1;
    double * v4 = &v3;
    double & v41 = v3;
    decltype(foo()) i = v3;
    decltype(v2) i2 = v2; // decltype one the var will return all the type (top const too)
    decltype(val_ptr) i3 = val_ptr;
    decltype(ci) i4 = ci;
    // decltype(ci) i4; // must initialize, as it is a ref
    cout << "7. decltype: " << i << endl;
    decltype(v4 + 0) v5;
    decltype(v41 + 0) v6; // this is type double, not double &
    // c++ set that if decltype get a de-ref, it will return a ref, (val) will also return a ref!
    // 如果 decltype(expr) 作用于一个左值表达式(a var that can set value to)，则推导为 T&
    // 如果 decltype(expr) 作用于一个右值表达式，则推导为 T(或 T&&)
    // decltype(*v4) v6; // double & type, same with last last comment, must initialize
    // decltype((v3)) v6; // double & type

    // 8. c type array
    int a[5] = {1, 2, 3, 4, 5};
    int *b1 = a;
    int (*b2)[5] = &a;
    int (&b3)[5] = a;
    auto ia2(a); // ia2 is a int * (a pointer), not a array, same as auto ia2 = a;
    decltype(a) ia3; // this is array int[5],  = {1, 2, 3, 4 , 5, 6, 7} is wrong, too many argument
    cout << "8. c type array : " << b1 << endl; // a is pointer to a[0], so does b
    cout << b2 << endl; // b2 is a pointer to int[5], so does &a
    cout << b3 << endl; // b3 == a, ref to int[5]
    cout << a << ", " << &a << ", " << *a << endl;

    // 9. iterator
    vector<int> vec1 = {1, 2, 3};
    const vector<int> vec2 = {1, 2, 3};
    auto vb = vec1.begin(); // this is of type std::vector<int>::iterator
    auto vb2 = vec2.begin(); // this is of type std::vector<int>::const_iterator
    cout << "9. iterator: " << *vb << endl;

    // 10. func ptr and call back func
    cout << "10. func ptr and call back func: " << endl;
    register_callback(call_back);
    void (*vec_fun_ptr)() = vector_erase_example;
    vec_fun_ptr();
    void (*fun_ptr_with_arg)(int a, int b) = test_fun;
    fun_ptr_with_arg(4, 5);

    // 11. pay attention to the c style 'string' function, or just don't use it.
    char ca[] = {'C','P','P'};
    // cout << strlen(ca) << endl; // undefined behavior, strlen will tries to find space

    // 12. define multiple pointer
    int* ak, bk; // bk is not pointer hhhhhhhh, you need to int* ak, *bk;

    cout << "Hello" << "\n";
    cout << "World" << "\r\n"; // it is the same in windows, maybe OS or compiler handles it
    // but in binary out put file, this may different
}

constexpr int get_size()
{
    return 1;
}

int foo()
{
    cout << "foo function" << endl;
    return 2;
}

void cin_example()
{
    cout << "\n cin knowledge: " << endl;
    int ii = 0;
    cin >> ii;
    cout << ii << endl;
    string name;
    cin >> name;
    cin.ignore(); // make the \n ignored
    cout << name << endl; // will only read firs string before space
    getline(cin, name); // will read all line
    cout << name << endl;
    char ch;
    cin.get(ch); // cin >> ch will skip space and \n
    cout << ch << endl;
    cin.tie(nullptr); // will untie the cin and cout, so their order will not garanteed, cout will not flush unless you do it manully
}

void vector_erase_example()
{
    vector<int> a = {1, 2, 3, 4 , 5, 6};
    // This is wrong, you can not erase in for loop
    // int i = 0;
    // for (auto & item : a)
    // { 
    //     if (item % 2 == 0) a.erase(a.begin() + i);
    //     i++;
    // }

    // Method 1
    // a.erase(std::remove_if(a.begin(), a.end(), [](int x) { return x % 2 == 0; }), a.end());

    // Method 2, iteration
    for (auto it = a.begin(); it != a.end(); ) {
        if (*it % 2 == 0)
            it = a.erase(it);  // `erase()` 返回下一个有效迭代器
        else
            ++it;
    }

    print_vector(a);
}

void print_vector(const vector<int> & vec, bool test_arg) // = true only one place 
{
    for (const auto &v : vec)
    {
        cout << v << " | ";
    }
    cout << endl;
}

void test_fun(int a, int b)
{
    cout << a + b << endl;
}

void call_back()
{
    cout << "call back" << endl;
}

void register_callback(void cb()) // same as void (*cb)()
{
    cout << "before call back" << endl;
    call_back();
    cout << "after call back" << endl;
}