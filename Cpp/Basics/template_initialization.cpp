#include <iostream>
#include "template_init_test.h"

// 模板定义
template <typename T>
void func(T x) {
    std::cout << "Value: " << x << std::endl;
}

// 显式实例化一部分类型
template void func<int>(int);      // 只显式实例化 int

int main() {
    func(42);      // 使用 int，正常工作（显式实例化）
    func(3.14);    // 使用 double，仍然工作（隐式实例化）
    template_test_fun(3);
    // template_test_fun(3.12); // error undefined reference to `void template_test_fun<double>(double)'
    template_test_fun2(3);
    template_test_fun2(3.1); // this works!
    return 0;
}