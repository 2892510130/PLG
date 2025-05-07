#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

int num = 1;
std::mutex mtx;
std::condition_variable cva;
std::condition_variable cvb;

void cond_test();
void safe_queue_test();

template<typename T>
class ThreadSafeQueue
{
    public:
        void push(T value)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_queue.push(value);
            m_cond.notify_one();
        }

        void wait_and_pop(T& value)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cond.wait(lock, [this]{return !m_queue.empty();});
            value = m_queue.front();
            m_queue.pop();
        }

        int size()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_queue.size();
        }

    private:
        std::mutex m_mtx;
        std::queue<T> m_queue;
        std::condition_variable m_cond;
};

int main()
{
    /*
        Use conditional var to create a safe queue.
        1. use condition_variable to notify and wait
    */

    cond_test();

    safe_queue_test();
}

void cond_test()
{
    std::thread t1([]() {
        for (int i = 0; i < 3; i++)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cva.wait(lock, []() {
                return num == 1;
            });

            num++;
            std::cout << "thread 1 print 1...\n";
            cvb.notify_one();
        }
    });

    std::thread t2([]() {
        for (int i = 0; i < 3; i++)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvb.wait(lock, []() {
                return num == 2;
            });

            num--;
            std::cout << "thread 2 print 2...\n";
            cva.notify_one();
        }
    });

    t1.join();
    t2.join();
}

void safe_queue_test()
{
    ThreadSafeQueue<int> safe_queue;
    std::mutex mtx_print;

    std::thread t2([&]() {
        for (int i = 0; i < 3; i++)
        {
            safe_queue.wait_and_pop(i);
            {
                std::lock_guard<std::mutex> lock(mtx_print);
                std::cout << "[pop] Size of the queue: " << safe_queue.size() << '\n';
            }
        }
    });

    std::thread t1([&]() {
        for (int i = 0; i < 3; i++)
        {
            safe_queue.push(i);
            {
                std::lock_guard<std::mutex> lock(mtx_print);
                std::cout << "[push] Size of the queue: " << safe_queue.size() << '\n';
            }
        }
    });

    t1.join();
    t2.join();
}