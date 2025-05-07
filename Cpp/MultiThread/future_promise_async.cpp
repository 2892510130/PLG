#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <future>

std::string fetch_data(std::string query)
{
    return "Data: " + query;
}

void use_package() {
    std::packaged_task<std::string(std::string)> task(fetch_data);
    std::future<std::string> result = task.get_future();
    std::thread t(std::move(task), "query");
    t.detach();

    std::string value = result.get();
    std::cout << "The result is: " << value << std::endl;
}

void set_value(std::promise<int>&& prom) 
{
    prom.set_value(10);
}

void set_exception(std::promise<void>&& prom) 
{
    try
    {
        throw std::runtime_error("An error occurred!");
    }
    catch(...)
    {
        prom.set_exception(std::current_exception());
    }
}

void test_promise_value()
{
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread t(set_value, std::move(prom));
    std::cout << "Waiting for the thread to set the value...\n";
    std::cout << "Value set by the thread: " << fut.get() << '\n';
    t.join();
}

void test_promise_exception()
{
    std::promise<void> prom;
    std::future<void> fut = prom.get_future();
    std::thread t(set_exception, std::move(prom));
    try
    {
        std::cout << "Waiting for the thread to set the exception...\n";
        fut.get();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception set by the thread: " << e.what() << '\n';
    }
    t.join();
}

void thread_func(std::shared_future<int> future)
{
    try
    {
        int result = future.get();
        std::cout << "Result: " << result << '\n';
    }
    catch(const std::exception& e)
    {
        std::cerr << "Future error: " << e.what() << '\n';
    }
}

void use_shared_future()
{
    std::promise<int> prom;
    std::shared_future<int> fut = prom.get_future();
    std::thread t1(set_value, std::move(prom));
    std::thread t2(thread_func, fut);
    std::thread t3(thread_func, fut);
    t1.join();
    t2.join();
    t3.join();
}

void block_async()
{
    std::cout << "Before block async.\n";
    {
        auto future = std::async(std::launch::async, []() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "std::async.\n";
        });
    }
    std::cout << "After block, not async, it is sequential!\n";
}

void dead_lock()
{
    std::mutex mtx;
    std::cout << "Main thread lock.\n";
    std::lock_guard<std::mutex> lock(mtx);
    auto future = std::async(std::launch::async, [&mtx]() {
        std::cout << "async called.\n";
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "async working...\n";
    });
    std::cout << "Dead lock end.\n";
}

int main()
{
    /*
        Async, Promise, Future
        1. async will open a thread and bind to a std::promise object, and can be get by a future
        2. get will block the thread until it get the result, it can only use once
            !! And deconstructor will also block the thread (call wait), see block_async.
        3. wait same as get but will not return result and it can use any times
        4. wait_for and wait_until to check whether the async is done for a timestep
        5. packaged_task can pack a taks on another thread and bind a future to get result
        6. promise set a value or exception at some thread, future get fom another thread,
            they are a pair, so when you free a promise you can not use the future (it will crash)
        7. shared_future, when you need multiple thread wait for a async task's result. (can call 
            get() multiple times)
        8. use try catch to get the future result.
        9. future will deconstruct when the last object holds it deconstruct, because in async
            we create a shared pointer of the future
    */
    std::future<std::string> result = std::async(std::launch::async, fetch_data, "Data");

    std::string data = result.get();
    std::cout << data << '\n';

    use_package();
    test_promise_value();
    test_promise_exception();
    use_shared_future();

    block_async();

    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // dead_lock();
}
