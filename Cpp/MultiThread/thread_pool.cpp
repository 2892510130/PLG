#include "thread_pool.h"
#include <chrono>

void print_hello(int num) {
    std::cout << "Hello " << num << " from thread " 
              << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int compute_sum(int a, int b) {
    return a + b;
}

int main()
{
    // static ThreadPool &tp = ThreadPool::get_instance();
    ThreadPool& pool = ThreadPool::get_instance(4);
    pool.commit(print_hello, 42);

    auto future = pool.commit(compute_sum, 10, 20);
    std::cout << "Sum result: " << future.get() << std::endl;

    std::vector<std::future<int>> results;
    for (int i = 0; i < 10; ++i) {
        results.push_back(pool.commit([i] { return i * i; }));
    }

    for (auto& fut : results) {
        std::cout << fut.get() << " ";
    }
    std::cout << '\n';

    ThreadPool& pool2 = ThreadPool::get_instance(2);
    std::vector<std::future<int>> results2;
    for (int i = 0; i < 4; ++i) {
        results2.push_back(pool2.commit([i] { return i * i; }));
    }
}