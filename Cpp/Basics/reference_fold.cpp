#include <type_traits>
#include <iostream>
#include <string>

static int i = 1;

template<typename T>
void check_reference(T&& x){
    std::cout << "---------------------------------------------\n";
    std::cout << i++ << '\n';
    std::cout << std::boolalpha;
    std::cout << "T is lvalue reference: " << std::is_lvalue_reference<T>::value << std::endl;
    std::cout << "T is rvalue reference: " << std::is_rvalue_reference<T>::value << std::endl;
    std::cout << "T is rvalue : " << std::is_rvalue_reference<T&&> ::value << std::endl;
    std::cout << "x is lvalue reference: " << std::is_lvalue_reference<decltype(x)>::value << std::endl;
    std::cout << "x is rvalue reference: " << std::is_rvalue_reference<decltype(x)>::value << std::endl;
}

void process(int&x){
    std::cout << "process(int&)" << std::endl;
}

void process(int&& x){
    std::cout << "process(int&&)" << std::endl;
}

// 通用的包装函数，使用转发引用
template<typename T>
void wrapper(T&& arg) {
    process(std::forward<T>(arg)); // 原样转发，T is int&& for 20, arg is T&& for 20, but ! arg is lvalue !
}

template<typename T>
void wrapper2(T&& arg) {
    process(arg); // 原样转发
}

int main() {
    int a = 10;
    check_reference(a);          // T被推导为 int&， 因此 T&& -> int& && -> int&
    check_reference(20);         // T被推导为 int，  T&& -> int&&
    int& c = a;
    check_reference(c);

    int&& b = 100;
    check_reference(b);
    check_reference(std::string({})); // same as std::move(b)

    wrapper(a);          // 传递左值，调用process(int&)
    wrapper(20);         // 传递右值，调用process(int&&)

    wrapper2(a);          // 传递左值，调用process(int&)
    wrapper2(20);         // 传递右值，arg 是 int&&类型，但arg本身是个左值
    return 0;
}