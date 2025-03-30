#pragma once
#include <iostream>

template <typename T>
void template_test_fun(T x);

template <typename T>
void template_test_fun2(T x)
{
    std::cout << "Value: " << x << std::endl;
}

template <>
inline void template_test_fun2(double x) // must has this inline, otherwise there will be multi def
{
    std::cout << "Double Value: " << x << std::endl;
}