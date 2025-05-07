#include <cassert>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

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

int main()
{
    /*
        Atomic and Memory Model
        1. only atomic operation has a certain Modification Order
        2. std::atomic_flag -> base of atomic, test_and_set() and clear()
        3. Memory Order: CPP has 6 of them.
            3.1 relaxed : only atomic, no sync and order
            3.2 consume : ensure dependent relation order
            3.3 acquire : no read write will put before it (for sync after read)
            3.4 release : no read write will put after it (for sync before write)
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
    */
    test_spin_lock();
    TestOrderRelaxed(); // sometimes will assert
    TestOderRelaxed2();
    return 0;
}
