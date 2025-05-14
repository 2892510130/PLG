#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <utility>
#include <atomic>

template<typename T, size_t Cap>
class CircularQueueLock : private std::allocator<T>
{
    public:
        CircularQueueLock() : m_size(Cap + 1), m_data(std::allocator<T>::allocate(m_size)) {}
        CircularQueueLock(const CircularQueueLock&) = delete;
        CircularQueueLock& operator=(const CircularQueueLock&) = delete;
        CircularQueueLock& operator=(const CircularQueueLock&) volatile = delete;

        ~CircularQueueLock()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            
            while (m_head != m_tail)
            {
                std::allocator<T>::destroy(m_data + m_head);
                m_head = (m_head + 1) % m_size;
            }
            
            std::allocator<T>::deallocate(m_data, m_size);
        }

        template<typename ...Args>
        bool emplace(Args && ... args)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if ((m_tail + 1) % m_size == m_head)
            {
                std::cout << "circular queue full!" << std::endl;
                return false;
            }

            std::allocator<T>::construct(m_data + m_tail, std::forward<Args>(args)...);
            m_tail = (m_tail + 1) % m_size;
            return true;
        }

        bool push(const T& val)
        {
            std::cout << "called push const T& version\n";
            return emplace(val);
        }

        bool push(T&& val)
        {
            std::cout << "called push T&& version\n";
            return emplace(std::move(val));
        }

        bool pop(T& val)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_head == m_tail)
            {
                std::cout << "circular queue empty!\n";
                return false;
            }

            val = std::move(m_data[m_head]);
            m_head = (m_head + 1) % m_size;
            return true;
        }

    private:
        size_t m_size;
        T* m_data;
        std::mutex m_mtx;
        size_t m_head = 0;
        size_t m_tail = 0;
};

template<typename T, size_t Cap>
class CircularQueueSeq : private std::allocator<T>
{
    public:
        CircularQueueSeq() : m_size(Cap + 1), m_data(std::allocator<T>::allocate(m_size)), m_atomic_suing(false) {}
        CircularQueueSeq(const CircularQueueSeq&) = delete;
        CircularQueueSeq& operator=(const CircularQueueSeq&) = delete;
        CircularQueueSeq& operator=(const CircularQueueSeq&) volatile = delete;

        ~CircularQueueSeq()
        {
            bool use_expected = false;
            bool use_desired = true;

            do
            {
                use_expected = false;
                use_desired = true;
            }
            while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));


            while (m_head != m_tail)
            {
                std::allocator<T>::destroy(m_data + m_head);
                m_head = (m_head + 1) % m_size;
            }
            
            std::allocator<T>::deallocate(m_data, m_size);

            do
            {
                use_expected = true;
                use_desired = false;
            }
            while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));
        }

        template<typename ...Args>
        bool emplace(Args && ... args)
        {
            bool use_expected = false;
            bool use_desired = true;

            do
            {
                use_expected = false;
                use_desired = true;
            }
            while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));

            if ((m_tail + 1) % m_size == m_head)
            {
                std::cout << "circular queue full!" << std::endl;
                do
                {
                    use_expected = true;
                    use_desired = false;
                }
                while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));
                return false;
            }

            std::allocator<T>::construct(m_data + m_tail, std::forward<Args>(args)...);
            m_tail = (m_tail + 1) % m_size;

            do
            {
                use_expected = true;
                use_desired = false;
            }
            while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));
            return true;
        }

        bool push(const T& val)
        {
            std::cout << "called push const T& version\n";
            return emplace(val);
        }

        bool push(T&& val)
        {
            std::cout << "called push T&& version\n";
            return emplace(std::move(val));
        }

        bool pop(T& val)
        {
            bool use_expected = false;
            bool use_desired = true;

            do
            {
                use_expected = false;
                use_desired = true;
            }
            while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));


            if (m_head == m_tail)
            {
                std::cout << "circular queue empty!\n";
                do
                {
                    use_expected = true;
                    use_desired = false;
                }
                while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));
                return false;
            }

            val = std::move(m_data[m_head]);
            m_head = (m_head + 1) % m_size;

            do
            {
                use_expected = true;
                use_desired = false;
            }
            while (!m_atomic_suing.compare_exchange_strong(use_expected, use_desired));
            return true;
        }

    private:
        size_t m_size;
        T* m_data;
        std::atomic<bool> m_atomic_suing;
        size_t m_head = 0;
        size_t m_tail = 0;
};

template<typename T, size_t Cap>
class CircularQueueLight : private std::allocator<T>
{
    public:
        CircularQueueLight() : m_size(Cap + 1), m_data(std::allocator<T>::allocate(m_size)), m_head(0), m_tail(0) {}
        CircularQueueLight(const CircularQueueLight&) = delete;
        CircularQueueLight& operator=(const CircularQueueLight&) = delete;
        CircularQueueLight& operator=(const CircularQueueLight&) volatile = delete;

        ~CircularQueueLight()
        {
            while (m_head.load() != m_tail.load())
            {
                size_t h = m_head.load();
                std::allocator<T>::destroy(m_data + h);
                m_head.compare_exchange_strong(h, (h + 1) % m_size, std::memory_order_release, std::memory_order_relaxed);
            }
            
            std::allocator<T>::deallocate(m_data, m_size);
        }

        template<typename ...Args>
        bool emplace(Args && ... args)
        {
            size_t t;

            do
            {
                t = m_tail.load(std::memory_order_relaxed); // use relaxed because we will check it again in while, if it's diff, do this again
                if ((t + 1) % m_size == m_head.load(std::memory_order_acquire))
                {
                    std::cout << "circular queue full!" << std::endl;
                    return false;
                }
            }
            while (!m_tail.compare_exchange_strong(t, (t + 1) % m_size, std::memory_order_release, std::memory_order_relaxed));

            std::allocator<T>::construct(m_data + t, std::forward<Args>(args)...);
            
            size_t tail_up;
            do
            {
                tail_up = t;
            }
            while (m_tail_update.compare_exchange_strong(tail_up, (tail_up + 1) % m_size, std::memory_order_release, std::memory_order_relaxed));
            return true;
        }

        bool push(const T& val)
        {
            std::cout << "called push const T& version\n";
            return emplace(val);
        }

        bool push(T&& val)
        {
            std::cout << "called push T&& version\n";
            return emplace(std::move(val));
        }

        bool pop(T& val)
        {
            size_t h;
            do
            {
                h = m_head.load(std::memory_order_relaxed);
                if (h == m_tail.load(std::memory_order_acquire))
                {
                    std::cout << "circular queue empty!\n";
                    return false;
                }

                if (h == m_tail_update.load(std::memory_order_acquire)) // for case tail++, but did not finish construct
                {
                    std::cout << "tail not updated!\n";
                    return false;
                }

                val = m_data[h]; // we can not use move, as maybe some thread will also be here, h is not the same (while cond not true)
            }
            while (!m_head.compare_exchange_strong(h, (h + 1) % m_size, std::memory_order_release, std::memory_order_relaxed));

            return true;
        }

    private:
        size_t m_size;
        T* m_data;
        std::atomic<size_t> m_head;
        std::atomic<size_t> m_tail;
        std::atomic<size_t> m_tail_update;
};

void test_seq()
{
    std::cout << "test seq\n";
    CircularQueueSeq<int, 4> cqs;

    std::thread t1([&]() {
        for (int i = 0; i < 3; i++)
        {
            cqs.push(i);
        }
    });

     std::thread t2([&]() {
        for (int i = 0; i < 3; i++)
        {
            int val;
            bool success = cqs.pop(val);
            if (success) std::cout << "val is " << val << '\n';
        }
    });

    t1.join();
    t2.join();
}


void test_light()
{
    std::cout << "test light\n";
    CircularQueueLight<int, 4> cql;

    std::thread t1([&]() {
        for (int i = 0; i < 3; i++)
        {
            cql.push(i);
        }
    });

     std::thread t2([&]() {
        for (int i = 0; i < 3; i++)
        {
            int val;
            bool success = cql.pop(val);
            if (success) std::cout << "val is " << val << '\n';
        }
    });

    t1.join();
    t2.join();
}

int main()
{
    /*
     *  No lock queue:
     *  1. push and pop will not effect efficency (push only wirte to tail, pop head, cycle queue strength!), so it is high concurrency.
     *  2. only for scalar var, not class, because we need to copy them (can not move) which takes time and memeory
     * */
    test_seq();

    test_light();
}
