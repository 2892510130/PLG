#include <iostream>
#include <thread>
#include <string>
#include <chrono>

void thread_work_1(std::string str);
void thread_work_2(std::string str);
void catch_exception();
void auto_guard();
void danger_ops(int _param);
void change_param(int &_param);
void ref_ops(int _param);
// void ref_ops_test(int &_param);
void bind_class_func();
void deal_unique(std::unique_ptr<int> p);
void move_oops();

class background_task
{
    public:
        void operator()(std::string str)
        {
            std::cout << "Use factor as the thread arg: " << str << '\n';
        }
};

struct func {
    int& _i;
    func(int & i): _i(i){}
    void operator()() 
    {
        std::cout << "_i is ";
        for (int i = 0; i < 3; i++) {
            _i = i;
            std::cout << _i << " ";
            // std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << '\n';
    }
};

class thread_guard {
    private:
        std::thread& _t;
    public:
        explicit thread_guard(std::thread& t):_t(t){}
        ~thread_guard() 
        {
            //join can only call once
            if (_t.joinable()) {
                _t.join();
            }
        }
    
        thread_guard(thread_guard const&) = delete;
        thread_guard& operator=(thread_guard const&) = delete;
};

class bind_class
{
    public:
        void do_some_work()
        {
            std::cout << "class func do some work\n";
        }
};

int main()
{
    /*
        Thread Basics:
        1.  use thread to open thread
        2.  use join to wait for the thread to complete
        3.  factor as the arg (also see catch_exception the func)
        4.  lambda as the arg
        5.  use detach to run in background
        6.  error handle : main thread crash, protect the sub thread.
            6.1 catch throw
            6.2 RAII way -- thread guard (not a lock guard)
        7.  be careful with the implicit convert (see function danger_ops)
        8.  reference param : must use std::ref
        9.  bind class func : must use &, and the arg is the &instance
        10. we can use move but can not copy
    */
    std::string hello_str = "Hello, thread!";
    std::thread t1(thread_work_1, hello_str);
    t1.join();

    std::thread t2(background_task(), hello_str);
    t2.join();

    std::thread t3([](std::string str) {std::cout << "Use lambda as the thread arg: " << str << '\n';}, hello_str);
    t3.join();

    catch_exception();

    auto_guard();

    ref_ops(3); // must use std::ref

    // int x = 3;
    // int &y = x;
    // ref_ops_test(y);

    bind_class_func();

    move_oops();

    danger_ops(4); // put at the end, as it use detach and we do not know when it will finish
}

void thread_work_1(std::string str)
{
    std::cout << str << '\n';
}

void thread_work_2(std::string str)
{
    std::cout << "Danger ops with implicit cast and detach: " << str << std::endl;
}

void catch_exception() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread  functhread{ myfunc };
    try 
    {
        std::cout << "Main thread do something\n";
    }
    catch (std::exception& e) 
    {
        functhread.join();
        throw;
    }

    functhread.join();
    std::cout << "local state is: " << some_local_state << '\n';
}

void auto_guard() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread  t(my_func);
    thread_guard g(t);
    std::cout << "auto guard finished " << std::endl;
}

void danger_ops(int _param)
{
    char buffer[1024];
    sprintf(buffer, "%i", _param);
    // std::thread t(thread_work_1, buffer); // if we use this and do not add a wait at the end, we will lose buffer in thread
    // code above only save buffer into the t member, and only when t starts to work buffer will send to the worker function
    std::thread t(thread_work_2, std::string(buffer)); // However this will not have the same problem, why?
    t.detach();
    std::cout << "dnager ops finished\n";
    // std::this_thread::sleep_for(std::chrono::seconds(1));
}

void change_param(int &_param)
{
    _param++;
}

void ref_ops(int _param)
{
    std::cout << "When work with ref, use std::ref as thread will do the forward: " << _param << " --> ";
    std::thread t(change_param, std::ref(_param)); // std::ref inside is also a right value but as a reference wrapper
    // this wrapper save the type and data in address, When we need to use the object, we just get the data in the address
    t.join();
    std::cout << _param << std::endl;
}

// void ref_ops_test(int &_param)
// {
//     std::cout << "When work with ref, use std::ref as thread will do the forward: " << _param << " --> ";
//     std::thread t(change_param, _param);
//     t.join();
//     std::cout << _param << std::endl;
// }

void bind_class_func()
{
    bind_class c;
    std::thread t(&bind_class::do_some_work, &c);
    t.join();
}

void deal_unique(std::unique_ptr<int> p) 
{
    std::cout << "unique ptr data is " << *p << std::endl;
    (*p)++;

    std::cout << "after unique ptr data is " << *p << std::endl;
}

void move_oops() {
    auto p = std::make_unique<int>(100);
    std::thread  t(deal_unique, std::move(p));
    t.join();
}