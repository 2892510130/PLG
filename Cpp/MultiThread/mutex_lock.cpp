#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <stack>
#include <memory>
#include <shared_mutex>
#include <map>

void func1();
void func2();
void unique_lock_func();
void own_lock(std::unique_lock<std::mutex> &lock);
void unique_defer_lock();
void use_own_defer();
void use_own_adopt();

std::mutex mtx1;
int shared_data = 0;

struct empty_stack : std::exception
{
    const char* what() const throw();
};


template<typename T>
class ThreadSafeStack
{
    private:
        std::stack<T> m_stack;
        mutable std::mutex m_mtx; // allow lock in const function
    
    public:
        ThreadSafeStack() {};

        ThreadSafeStack(const ThreadSafeStack &other)
        {
            std::lock_guard<std::mutex> lock(other.m_mtx);
            m_stack = other.m_stack;
        }

        ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;

        void push(T value)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_stack.push(std::move(value));
        }

        void pop(T &value)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_stack.empty()) throw empty_stack();
            value = m_stack.top();
            m_stack.pop();
        }

        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            // if (m_stack.empty()) throw empty_stack();
            if (m_stack.empty()) return nullptr;
            std::shared_ptr<T> pointer(std::make_shared<T>(m_stack.top()));
            m_stack.pop();
            return pointer;
        }
};

class HierarchicalMutex
{
    private:
        std::mutex m_internal_mutex;
        unsigned long const m_hierarchy_value;
        unsigned long m_previoue_hierarchy_value;
        static thread_local unsigned long m_this_thread_hierarchy_value;

        void check_hierarchy_violation()
        {
            if (m_this_thread_hierarchy_value <= m_hierarchy_value)
            {
                throw std::logic_error("mutex hierarchy violated");
            }
        }

        void update_hierarchy_value()
        {
            m_previoue_hierarchy_value = m_this_thread_hierarchy_value;
            m_this_thread_hierarchy_value = m_hierarchy_value;
        }
    
    public:
        explicit HierarchicalMutex(unsigned long value) : m_hierarchy_value(value), m_previoue_hierarchy_value(0) {};
        HierarchicalMutex(const HierarchicalMutex &other) = delete;
        HierarchicalMutex& operator=(const HierarchicalMutex &) = delete;

        void lock()
        {
            check_hierarchy_violation();
            m_internal_mutex.lock();
            update_hierarchy_value();
        }

        void unlock()
        {
            if (m_this_thread_hierarchy_value != m_hierarchy_value)
            {
                throw std::logic_error("mutex hierarchy violated");
            }

            m_this_thread_hierarchy_value = m_previoue_hierarchy_value;
            m_internal_mutex.unlock();
        }

        bool try_lock()
        {
            check_hierarchy_violation();
            if (m_internal_mutex.try_lock())
            {
                return false;
            }
            update_hierarchy_value();
            return true;
        }
};

thread_local unsigned long HierarchicalMutex::m_this_thread_hierarchy_value(ULONG_MAX);

class DNSService
{
    public:
        DNSService() {};
        std::string query(std::string dns_name) // read use shared lock
        {
            std::shared_lock<std::shared_mutex> shared_lock(m_shared_mtx);
            auto iter = m_dns_info.find(dns_name);
            if (iter != m_dns_info.end())
            {
                return iter->second;
            }
            return "";
        }

        void add_dns_info(std::string dns_name, std::string dns_entry) // write use owned lock
        {
            std::lock_guard<std::shared_mutex> guard_lock(m_shared_mtx);
            m_dns_info.insert(std::make_pair(dns_name, dns_entry));
        }
    
        private:
            std::map<std::string, std::string> m_dns_info;
            mutable std::shared_mutex m_shared_mtx;
};

int main()
{
    /*
        Exclusion and Lock:
        1. lock and unlock yourself, or use lock guard
        2. thread safe data (stack), when pop check empty and return nullptr or throw
        3. dead lock : thread 1 first lock A then do something then lock B, thread 2 first lock B then lock A
            3.1 only lock and unlock one mutex
            3.2 but sometimes we need two lock in one function
                3.2.1 lock at the same time : lock A and lock B then do something, 
                    std::lock(mtx1, mtx2) and use guard to adopt these 2 mutex.
                3.2.2 the same before, but use std::scoped_lock guard(mtx1, mtx2). 
                3.2.3 use hierarchical mutex, avoid loop lock, first lock higher weight lock
        4. unique_lock : same as lock_guard but we can manually unlock it.
            4.1 std::defer_lock can defer the lock
            4.2 std::adopt_lock to adopt the lock and owns_lock will return true
            4.3 mutex can not be moved but unique_lock can
            4.4 some func or operation does not access to the shared data, do not put them into lock, and unique_lock is good
                for this because it can unlock manually
        5. shared_lock : read can be done by muliple threads, they are not exclusive. read and write is, write and write is.
            so read needs shared lock, and write need unique (owned) lock.
            5.1 shared_mutex and shared_timed_mutex (will set a timeout, if in this timeout it can not get lock return false)
            5.2 use shared_lock to hold shared_mutex for read
            5.3 use lock_guard to hold shared_mutex for write
        6. recursive lock : sometimes we need lock something and into another interface (see func use_own_defer) and stack
            here we can use recursive lock.
            6.1 However we should avoid this, from a design level try to lock the interface altogether
    */
    std::thread t1(func1);
    std::thread t2(func2);
    t1.join();
    t2.join();

    unique_lock_func();
    unique_defer_lock();

    use_own_adopt();

    // use_own_defer();
}

void func1()
{
    for (int i = 0; i < 3; i++)
    {
        mtx1.lock();
        shared_data++;
        std::cout << "Thread id : " << std::this_thread::get_id() << '\n';
        std::cout << "Shared data : " << shared_data << '\n';
        mtx1.unlock();
    }
}

void func2()
{
    for (int i = 0; i < 3; i++)
    {
        std::lock_guard<std::mutex> lock(mtx1);
        shared_data++;
        std::cout << "Thread id : " << std::this_thread::get_id() << '\n';
        std::cout << "Shared data : " << shared_data << '\n';
    }
}

void unique_lock_func()
{
    std::unique_lock<std::mutex> lock(mtx1);
    own_lock(lock);

    lock.unlock();
    own_lock(lock);
}

void own_lock(std::unique_lock<std::mutex> &lock)
{
    if (lock.owns_lock())
    {
        std::cout << "Owns lock.\n";
    }
    else
    {
        std::cout << "Owns no lock.\n";
    }
}

void unique_defer_lock()
{
    std::unique_lock<std::mutex> lock(mtx1, std::defer_lock);
    own_lock(lock);
    lock.lock();
    lock.unlock();
}

void use_own_adopt()
{
    mtx1.lock();
    std::unique_lock<std::mutex> lock(mtx1, std::adopt_lock);
    own_lock(lock);
    lock.unlock();
}


void use_own_defer() 
{
    std::unique_lock<std::mutex>  lock(mtx1);

    if (lock.owns_lock())
    {
        std::cout << "Main thread has the lock." << std::endl;
    }
    else
    {
        std::cout << "Main thread does not have the lock." << std::endl;
    }

    std::thread t([]() {
        std::unique_lock<std::mutex> lock(mtx1, std::defer_lock);

        if (lock.owns_lock())
        {
            std::cout << "Thread has the lock." << std::endl;
        }
        else
        {
            std::cout << "Thread does not have the lock." << std::endl;
        }

        lock.lock(); // will stack as the main thread does not unlock the lock

        if (lock.owns_lock())
        {
            std::cout << "Thread has the lock." << std::endl;
        }
        else
        {
            std::cout << "Thread does not have the lock." << std::endl;
        }

        lock.unlock();
        });
    t.join();
}