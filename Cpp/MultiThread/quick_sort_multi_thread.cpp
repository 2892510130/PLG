#include <iostream>
#include <list>
#include <thread>
#include <future>
#include <algorithm>
#include "thread_pool.h"

template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if (input.empty())
    {
        return input;
    } 

    std::list<T> result;
    result.splice(result.begin(), input, input.begin()); // get the first element to the result

    T const &pivot = *result.begin(); // use STL func partion to partion the list
    auto divide_point = std::partition(input.begin(), input.end(), [&](T const &t) {return t < pivot;});

    std::list<T> lower_part;
    lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);

    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template<typename T>
void sequential_quick_sort_inplace(std::list<T> &input)
{
    if (input.empty() || input.size() == 1)
    {
        return;
    } 

    auto pivot_it = input.begin();
    T pivot = *pivot_it;

    auto divide_point = std::partition(std::next(input.begin()), input.end(), [&](T const &t) {return t < pivot;});

    auto new_pivot = std::prev(divide_point);
    std::iter_swap(pivot_it, new_pivot);

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    std::list<T> higher_part;
    higher_part.splice(higher_part.end(), input, divide_point, input.end());

    sequential_quick_sort_inplace(lower_part);
    sequential_quick_sort_inplace(higher_part);

    input.splice(input.begin(), lower_part);
    input.splice(input.end(), higher_part);
}

template<typename T>
void parallel_quick_sort_inplace(std::list<T> &input)
{
    if (input.empty() || input.size() == 1)
    {
        return;
    } 

    auto pivot_it = input.begin();
    T pivot = *pivot_it;

    auto divide_point = std::partition(std::next(input.begin()), input.end(), [&](T const &t) {return t < pivot;});

    auto new_pivot = std::prev(divide_point);
    std::iter_swap(pivot_it, new_pivot);


    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    std::list<T> higher_part;
    higher_part.splice(higher_part.end(), input, divide_point, input.end());

    auto future_lower = std::async(std::launch::async, [&lower_part]() { sequential_quick_sort_inplace(lower_part); });
    sequential_quick_sort_inplace(higher_part);

    future_lower.get();

    input.splice(input.begin(), lower_part);
    input.splice(input.end(), higher_part);
}

template<typename T>
void thread_pool_quick_sort(std::list<T> &input)
{
    if (input.empty() || input.size() == 1)
    {
        return;
    } 

    auto pivot_it = input.begin();
    T pivot = *pivot_it;

    auto divide_point = std::partition(std::next(input.begin()), input.end(), [&](T const &t) {return t < pivot;});

    auto new_pivot = std::prev(divide_point);
    std::iter_swap(pivot_it, new_pivot);


    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    std::list<T> higher_part;
    higher_part.splice(higher_part.end(), input, divide_point, input.end());

    auto future_lower = ThreadPool::get_instance().commit([&lower_part]() { sequential_quick_sort_inplace(lower_part); });
    // auto future_lower = std::async(std::launch::async, [&lower_part]() { sequential_quick_sort_inplace(lower_part); });
    sequential_quick_sort_inplace(higher_part);

    future_lower.get();

    input.splice(input.begin(), lower_part);
    input.splice(input.end(), higher_part);
}

int main()
{
    /*
        parallel sort:
        1. sequential sort
        2. sequential sort inplace
        3. parallel sort inplace
        4. parallel (with thread pool) sort inplace
    */
    std::list<int> num_list = {6, 1, 0, 7, 5, 2, 9, -1};
    auto sorted = sequential_quick_sort(num_list);
    for (auto &d : sorted)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';

    sequential_quick_sort_inplace(num_list);
    for (auto &d : num_list)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';

    std::list<int> num_list_2 = {6, 1, 0, 7, 5, 2, 9, -1};
    parallel_quick_sort_inplace(num_list_2);
    for (auto &d : num_list_2)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';

    sequential_quick_sort_inplace(num_list);
    for (auto &d : num_list)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';

    parallel_quick_sort_inplace(num_list);
    for (auto &d : num_list)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';

    std::list<int> num_list_3 = {6};
    sequential_quick_sort_inplace(num_list_3);
    for (auto &d : num_list_3)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';

    std::list<int> num_list_4 = {6, 1, 0, 7, 5, 2, 9, -1};
    thread_pool_quick_sort(num_list_4);
    for (auto &d : num_list_4)
    {
        std::cout << d << " | ";
    }
    std::cout << '\n';
}
