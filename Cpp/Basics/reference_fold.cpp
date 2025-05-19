#include <type_traits>
#include <boost/type_index.hpp>
#include <iostream>
#include <future>
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

using boost::typeindex::type_id_with_cvr;

int functionint(int param) {
    std::cout << "param is " << param << std::endl;
    return 0;
}

template <class F, class... Args>
std::future<int> commit(F&& f, Args&&... args) {
    //....
        // 利用Boost库打印模板推导出来的 T 类型
    std::cout << "F type：" << type_id_with_cvr<F>().pretty_name() << std::endl;

    // 利用Boost库打印形参的类型
    std::cout << "f type:" << type_id_with_cvr<decltype(f)>().pretty_name() << std::endl;

    std::cout << "Args type：" << type_id_with_cvr<Args...>().pretty_name() << std::endl;

    std::cout << "args type：" << type_id_with_cvr<decltype(args)...>().pretty_name() << std::endl;

    return std::future<int>();
}

void reference_collapsing(){
    int a = 3;
    commit(functionint, a);
}

void reference_2()
{
    commit(std::move(functionint), 3);
}

template<typename T>
void use_tempref(T&& tparam) {
    int a = 4;
    tparam = a;
    tparam = std::move(a);
    std::cout << tparam << '\n';
}

void test_tempref() {
    use_tempref(3);
}

// void use_rightref(int && rparam) { // This will cause error, because it needs a right ref, but a left value in
// }

// template<typename T>
// void use_tempref2(T&& tparam) { // tparam is of type T&&, right ref, but it is left value!
//     use_rightref(tparam);
// }

// void test_tempref2() {
//     use_tempref2(3);
// }

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

    reference_collapsing();
    reference_2();
    test_tempref();
    
    return 0;
}