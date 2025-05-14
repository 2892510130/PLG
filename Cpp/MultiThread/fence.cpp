#include <cassert>
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x_then_y_fence()
{
    x.store(true, std::memory_order_relaxed);  //1
    std::atomic_thread_fence(std::memory_order_release);  //2, make sure 1 before 2
    y.store(true, std::memory_order_relaxed);  //3
}

void read_y_then_x_fence()
{
    while (!y.load(std::memory_order_relaxed));  //4
    std::atomic_thread_fence(std::memory_order_acquire); //5, make sure 6 read after 5
    if (x.load(std::memory_order_relaxed))  //6
        ++z;
}

void TestFence()
{
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y_fence);
    std::thread b(read_y_then_x_fence);
    a.join();
    b.join();
    assert(z.load() != 0); // 7
}

int main()
{
    TestFence();
}
