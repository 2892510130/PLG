#include <iostream>
#include <thread>
#include <mutex>

class SingletonEasy
{
    public:
        static SingletonEasy &get_inst()
        {
            static SingletonEasy single;
            return single;
        }
    
    private:
        SingletonEasy() {};
        SingletonEasy(const SingletonEasy&) = delete;
        SingletonEasy& operator=(const SingletonEasy&) = delete;
};

template<typename T>
class SafeDeletor
{
    public:
        void operator()(T* ptr)
        {
            std::cout << "We delete a singleton pointer!\n";
            delete ptr;
        }
};

template<typename T>
class Singleton
{
    protected:
        Singleton() = default;
        ~Singleton() = default;
        Singleton(const Singleton &) = delete;
        Singleton& operator=(const Singleton &) = delete;
        static std::shared_ptr<T> m_instance;
    
    public:
        static std::shared_ptr<T> get_instance()
        {
            static std::once_flag s_flag;
            std::call_once(s_flag, [&]() {
                m_instance = std::shared_ptr<T>(new T, SafeDeletor<T>());   
            });
            return m_instance;
        }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::m_instance = nullptr;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>; // allow Singleton<LogicSystem> access the LogicSystem private constructor
    LogicSystem() {};
};

class NumericSystem : public Singleton<NumericSystem>
{
    friend class Singleton<NumericSystem>;
    NumericSystem() {};

    public:
        int numeric = 0;
};

void test_singleton();

int main()
{
    /*
        Thread safe singleton
        1. local static var : from c++ 11 it is safe
        2. hunger mode initialize the singleton from main thread once
        3. lazy mode use mutex, but we don't know which thread initialze it, so the release is a problem
        4. the release problem can be deal with smart pointer, however still 2 problem
            4.1 we don't know whether will someone delete the ptr by hand, so we can add a helper class to do
                the delete.
            4.1 single = std::shared_ptr<Singleton>(new Singleton, SafeDeletor()) is not atomic, so we maybe
                first initialize single with it's memory but does not do the constructor at some time point,
                so if some thread try to get single, there will be a problem
        5. call_once can fix this
        6. use inherit and template to create multiple singleton class
    */

    auto logic_single = LogicSystem::get_instance();
    std::thread t1(test_singleton);
    std::thread t2(test_singleton);
    t1.join();
    t2.join();
    auto numeric_single = NumericSystem::get_instance();
    std::cout << numeric_single->numeric << '\n';
}

/* Lazy mode
static SinglePointer* GetInst()
{
    if (single != nullptr)
    {
        return single;
    }
    s_mutex.lock();
    if (single != nullptr)
    {
        s_mutex.unlock();
        return single;
    }
    single = new SinglePointer();
    s_mutex.unlock();
    return single;
}
*/

void test_singleton()
{
    auto numeric_single = NumericSystem::get_instance();
    numeric_single->numeric++;
}