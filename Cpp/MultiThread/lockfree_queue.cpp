#include <thread>

#include "lockfree_queue.h"

void TestLockFreeQueMultiPushPop(int test_count)
{
    LockFreeQueue<int> que;
    std::thread t1([&]()
                   {
        for (int i = 0; i < test_count; i++) {
            que.push(i);
            // std::cout << que.construct_count.load() << "---------------------" << que.destruct_count.load() << '\n';
            std::cout << "push data is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } });

    std::thread t4([&]()
                   {
        for (int i = test_count; i < test_count * 2; i++) {
            que.push(i);
            // std::cout << que.construct_count.load() << "---------------------" << que.destruct_count.load() << '\n';
            std::cout << "push data is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } });

    std::thread t2([&]()
                   {
        for (int i = 0; i < test_count;) {
            auto p = que.pop();
            // std::cout << que.construct_count.load() << "---------------------" << que.destruct_count.load() << '\n';
            if (p == nullptr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            i++;
            std::cout << "pop data is " << *p << std::endl;
        } });

    std::thread t3([&]()
                   {
        for (int i = 0; i < test_count;) {
            auto p = que.pop();
            // std::cout << que.construct_count.load() << "---------------------" << que.destruct_count.load() << '\n';
            if (p == nullptr) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            i++;
            std::cout << "pop data is " << *p << std::endl;
        } });

    t1.join();
    t4.join();

    t2.join();
    t3.join();
    std::cout << std::endl;

    std::cout << que.construct_count << "---------------------" << que.destruct_count << '\n';
}

int main(int argc, char **argv)
{
    /*
     *   Lock Free Queue:
     *   1. compare with lock_free_stack, now we have three counter, and each node has a atomic node_ptr.
     *      Because stack only need to handle head, but queue need to handle tail too.
     * */
    int test_count = 3;
    if (argc == 1)
        std::cout << "Missing test number, default it to 3\n";
    else if (argc == 2)
        test_count = std::atoi(argv[1]);
    TestLockFreeQueMultiPushPop(test_count);
}
