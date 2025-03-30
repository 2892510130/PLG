#include <iostream>
#include "template_init_test.h"

template <typename T>
void template_test_fun(T x) {
    std::cout << "Value: " << x << std::endl;
}

template void template_test_fun<int>(int); 

template void template_test_fun2<int>(int); 