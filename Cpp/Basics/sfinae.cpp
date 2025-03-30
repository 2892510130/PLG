#include <type_traits>
#include <iostream>

// 辅助类型，检测是否存在成员函数 foo
template <typename T>
class has_foo {
private:
    typedef char yes[1];
    typedef char no[2];

    template <typename U, void (U::*)()>
    struct SFINAE {};

    template <typename U>
    static yes& test(SFINAE<U, &U::foo>*);

    template <typename U>
    static no& test(...);

public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes);
};

// 函数仅在 T 有 foo() 成员时启用
template <typename T>
typename std::enable_if<has_foo<T>::value, void>::type
call_foo(T& obj) {
    obj.foo();
    std::cout << "foo() called." << std::endl;
}

class WithFoo {
public:
    void foo() { std::cout << "WithFoo::foo()" << std::endl; }
};

class WithoutFoo {};

int main() {
    WithFoo wf;
    call_foo(wf); // 输出: WithFoo::foo() \n foo() called.

    // WithoutFoo wf2;
    // call_foo(wf2); // 编译错误，没有匹配的函数
    return 0;
}