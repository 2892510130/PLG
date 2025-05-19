#include <set>
#include <mutex>
#include <iostream>
#include <thread>

#include "ref_count_stack.h"

void TestSingleRefStack() {
    RefCountStack<int>  single_ref_stack;
    std::atomic<int> cnt{0};

    std::thread t1([&]() {
        for (int i = 0; i < 20; i++) {
            single_ref_stack.push(i);
            std::cout << "push data " << i << " success!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        });

    std::thread t2([&]() {
        for (int i = 0; i < 10;) {
            auto head = single_ref_stack.pop();
            if (!head) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            i++;
            cnt.fetch_add(1);
        }
        });

    std::thread t3([&]() {
        for (int i = 0; i < 10;) {
            auto head = single_ref_stack.pop();
            if (!head) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            i++;
            cnt.fetch_add(1);
        }
        });

    t1.join();
    t2.join();
    t3.join();

    std::cout << "Poped " << cnt.load() << " data\n";
}

int main()
{
    /*
     *   Ref Count Lock-free Stack:
     *   1. g++ .\ref_count_lockfree_stack.cpp -o ref_count_lockfree_stack -Wall -Wextra -latomic [must have this],
     *      the atomic of the struct is with one int and one pointer, which takes 128 bits. And this need atomic lib.
     *   2. in atomic store the struct, not the pointer. As we do not want different threads work on same memory, in 
     *      this way, we create copy, and will not effect other thread.
     *   3. use two counter, external_count count the head (counted_node_ptr) threads, and internal_count count the 
     *      count_node. Only one will have problem, see the comment below, the code with problem.
     *   4. some explaination: t1 with head{p1, external_count=1}, we get to 1 before 2, with head{p1, external_count=2},
     *      then t2 get in push one node, head{p2, external_count=1}, for t1 old_head{p1, external_count=2}, compare in 2
     *      will not match, so go to else if, p1->internal_count = -1, we do not delete the p1. Then another thread pop 
     *      p2 successfully. Then t3 came with old_head{p1, external_count=3} (origin 2 does not decrease), it get into 2
     *      swap the data, internal_count = 3 - 2 = 1. p1->internal_count = -1 == -internal_count, so delete the ptr. No 
     *      memory leak!
     * */
    TestSingleRefStack();
}

/*
 std::shared_ptr<T> pop() {
    //0 处
    ref_node* old_head = head.load();
    for (;;) {
        //1 只要执行pop就对引用计数+1并更新到head中
        ref_node* new_head;
        do {
            new_head = old_head;
            //7 处
            new_head->_ref_count += 1;
        } while (!head.compare_exchange_weak(old_head, new_head));
        //4 
        old_head = new_head;

        auto* node_ptr = old_head->_node_ptr;
        if (node_ptr == nullptr) {
            return  std::shared_ptr<T>();
        }

        //2 比较head和old_head想等则交换否则说明head已经被其他线程更新
        if (head.compare_exchange_strong(old_head, node_ptr->_next)) {
                
            //要返回的值
            std::shared_ptr<T> res;
            //交换智能指针
            //5 处
            res.swap(node_ptr->_data);

            //6 增加的数量
            int increase_count = old_head->_ref_count.fetch_sub(2);
                
            //3 处判断仅有当前线程持有指针则删除
            if (increase_count == 2) {
                delete node_ptr;
            }

            return res;
        }else {
            if (old_head->_ref_count.fetch_sub(1) == 1) {
                delete node_ptr;
            }
        }
    }
}
 * */
