#include <cassert>
#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

class SpinLock
{
    public:
        void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire));
        }

        void unlock()
        {
            flag.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

void test_spin_lock()
{
    SpinLock spin_lock;
    std::thread t1([&spin_lock]() {
        spin_lock.lock();
        for (int i = 0; i < 3; i++)
        {
            std::cout << "*";
        }
        std::cout << '\n';
        spin_lock.unlock();
    });

    std::thread t2([&spin_lock]() {
        spin_lock.lock();
        for (int i = 0; i < 3; i++)
        {
            std::cout << "?";
        }
        std::cout << '\n';
        spin_lock.unlock();
    });

    t1.join();
    t2.join();
}

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);  // 1
    y.store(true, std::memory_order_relaxed);  // 2
}

void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed)) { // 3
        std::cout << "y load false" << std::endl;
    }

    if (x.load(std::memory_order_relaxed)) { //4
        ++z;
    }

}

void TestOrderRelaxed() {
    std::thread t1(write_x_then_y);
    std::thread t2(read_y_then_x);
    t1.join();
    t2.join();
    assert(z.load() != 0); // 5
}

void TestOderRelaxed2() {
    std::atomic<int> a{ 0 };
    std::vector<int> v3, v4;
        std::thread t1([&a]() {
            for (int i = 0; i < 10; i += 2) {
                a.store(i, std::memory_order_relaxed);
            }	
        });

        std::thread t2([&a]() {
            for (int i = 1; i < 10; i += 2)
                a.store(i, std::memory_order_relaxed);
            });


        std::thread t3([&v3, &a]() {
            for (int i = 0; i < 10; ++i)
                v3.push_back(a.load(std::memory_order_relaxed));
            });

        std::thread t4([&v4, &a]() {
            for (int i = 0; i < 10; ++i)
                v4.push_back(a.load(std::memory_order_relaxed));
            });

        t1.join();
        t2.join(); 
        t3.join(); 
        t4.join();

        for (int i : v3) {
            std::cout << i << " ";
        }
            
        std::cout << std::endl;
        for (int i : v4) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
}

void test_release_acquire()
{
    // 3 is before 2, and 1 is before 2, so 1 happens-before 3 and 1 happens-before 4
    std::atomic<bool> rx, ry;

    std::thread t1([&]() {
        rx.store(true, std::memory_order_relaxed); // 1
        ry.store(true, std::memory_order_release); // 2 make sure all operations (this thread) before happens-before (in other threads' acquire)
    });

    std::thread t2([&]() {
        while (!ry.load(std::memory_order_acquire)); // 3 make sure happens-before all operations after
        assert(rx.load(std::memory_order_relaxed));  // 4
    });

    t1.join();
    t2.join();
}

void release_sequence()
{
    // 5 is synchronizes-with 2, some operations between them which is a release sequence
    std::vector<int> data;
    std::atomic<int> flag{0};

    std::thread t1([&]() {
        data.push_back(42); // 1
        flag.store(1, std::memory_order_release); // 2, must happens-before 4
    });

    std::thread t2([&]() {
        int expected = 1; // 3, compare_exchange_strong will change flag to 2 if flag == expected
        while (!flag.compare_exchange_strong(expected, 2, std::memory_order_relaxed)) expected = 1; // 4, must happens-before 5
    });

    std::thread t3([&]() {
        while (flag.load(std::memory_order_acquire) < 2); // 5
        assert(data.at(0) == 42); // 6
    });

    t1.join();
    t2.join();
    t3.join();
}

void consume_dependency()
{
    std::atomic<std::string*> ptr;
    int data;

    std::thread t1([&]() {
        std::string* p = new std::string("Hello World");
        data = 42;
        ptr.store(p, std::memory_order_release);
    });

    std::thread t2([&]() {
        std::string* p2;
        while (!(p2 = ptr.load(std::memory_order_consume)));
        assert(*p2 == "Hello World");
        assert(data == 42);
    });

    t1.join();
    t2.join();
}

// if we do not use call once, we can use memory model to handle new instance problem
template<typename T>
class Singleton
{
    protected:
        Singleton() = default;
        ~Singleton() = default;
        Singleton(const Singleton &) = delete;
        Singleton& operator=(const Singleton &) = delete;
        static std::shared_ptr<T> m_instance;
        static std::mutex m_mtx;
        static std::atomic<bool> m_init;
    
    public:
        static std::shared_ptr<T> get_instance()
        {
            if (m_init.load(std::memory_order_acquire))
            {
                return m_instance;
            }

            m_mtx.lock();

            if (m_init.load(std::memory_order_relaxed))
            {
                m_mtx.unlock();
                return m_instance;
            }

            m_instance = std::shared_ptr<T>(new T);
            m_init.store(true, std::memory_order_release);
            m_mtx.unlock();
            return m_instance;
        }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::m_instance = nullptr;


int main()
{
    /*
        Atomic and Memory Model
        1. only atomic operation has a certain Modification Order
        2. std::atomic_flag -> base of atomic, test_and_set() and clear()
        3. Memory Order: CPP has 6 of them.
            3.1 relaxed : only atomic, no sync and order
            3.2 consume : ensure dependent relation order
            3.3 acquire : no read write will put before it (for sync after read)  [mutex lock]
            3.4 release : no read write will put after it (for sync before write) [mutex unlock]
            3.5 acq_rel : acquire and release
            3.6 seq_cst : most restrict order, global in order operation
            store() can use 1, 4, 6; load() can use 1, 2, 3, 6; read-modify-write can use all.
            atomic operation default use 6.
        4. Spin Lock: loop to check the locked mutex if it is locked by others.
        5. Memory Model details:
            5.1 in pc we have multiple cores, each core has it's own store buffer (1th cache),
                2 cpu share a Cache (2th cache). They all share a memory (3th cache).
            5.2 so how can we ensure the data consistance? MESI protocal, it is a Write Invalidate
                protocal. Only 1 CPU can wirte, other read, after write to Cache it will broadcast
                a "Invalidate" to all other CPU cores. MESI has 4 label:
                5.2.1 M (modified) other cpu can read the data to Cache
                5.2.2 E (exclusive) only related to this cpu store buffer, can freely read and write
                5.2.3 S (shared) if we want to change this, we must first broadcast, make all other 
                    cpu cache (this) data invalidate, update them. Only 1 cpu can wirte to the data,
                    !!HOWEVER!! all cpus can read, so they may read the old data (broadcast not yet
                    come), so we need to introduce Memory Order.
                5.2.4 I (invalidate)
            5.3 more about Memory Order: 
                5.3.1 two term "synchronizes-with", A ~ B means B read after A. "happens-before" (more 
                than "synchronizes-with").
                5.3.2 relaxed: only one atomic var in one thread is "happens-before", other condition
                    do not. So it is most relaxed one. Do not "synchronizes-with".
                5.3.3 acquire on the release will create a "synchronizes-with" relation, see test_release_acquire,
                    all the operations before release will be seen by operations after acquire.
                    for multiple threads release on same var and other thread acquire, only one release synchronizes-with
                    this acquire!!!
                5.3.4 this relation does not only happen immediatly, it can happen with a sequence, see release_sequence()
                5.3.5 consume on the release will create a "dependency-order-before" relation, not so common,
                    only dependency keep order, modern compiler often turns it to acquire. Use in no lock structure.
            5.4 one way to handle new singleton problem without call once
    */
    test_spin_lock();
    TestOrderRelaxed(); // sometimes will assert
    TestOderRelaxed2();

    test_release_acquire();
    return 0;
}
