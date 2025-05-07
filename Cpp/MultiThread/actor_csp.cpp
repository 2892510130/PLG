#include <chrono>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

template<typename T>
class Channel
{
    private:
        std::queue<T> m_queue;
        std::mutex m_mtx;
        std::condition_variable m_producer;
        std::condition_variable m_consumer;
        size_t m_capacity;
        bool m_closed = false;

    public:
        Channel(size_t capacity = 0) : m_capacity(capacity) {}

        bool send(T value)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_producer.wait(lock, [this]() {
                return (m_capacity == 0 && m_queue.empty()) || m_queue.size() < m_capacity || m_closed;
            });

            if (m_closed) return false;

            m_queue.push(value);
            m_consumer.notify_one();
            return true;
        }

        bool receive(T& value)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_consumer.wait(lock, [this]() {
                return !m_queue.empty() || m_closed;
            });

            if (m_closed && m_queue.empty()) return false;

            value = m_queue.front();
            m_queue.pop();
            m_producer.notify_one();
            return true;
        }

        void close()
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_closed = true;
            m_producer.notify_all();
            m_consumer.notify_all();
        }
};

int main()
{
    /*
        Actor and CSP : 2 design pattern
        Do not communicate with shared memory, share memory with communicating!!
        1. actor does not need shared value, it only process one message one time, so it is safe.
            It knows who will receive and who give the message. It may not need lock.
        2. CSP (communicating sequential process) : focus on the channel, know nothing about recevier and giver.
        3. In the blog, there is a atm example, a lot of codes, do not see then all. [about Actor implements]
    */
    Channel<int> ch(10);

    std::thread producer([&]() {
        for (int i = 0; i < 5; ++i)
        {
            ch.send(i);
            std::cout << "Sent: " << i << "\n";
        }
        ch.close();
    });

    std::thread consumer([&]() {
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        int val;
        while (ch.receive(val)) 
        {
            std::cout << "Received: " << val << '\n';
        }
    });

    producer.join();
    consumer.join();
}
