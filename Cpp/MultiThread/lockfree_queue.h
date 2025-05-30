#pragma once

#include <iostream>
#include <atomic>
#include <memory>

template <typename T>
class LockFreeQueue
{
private:
    struct node_counter
    {
        unsigned internal_count : 30; // field, 30 bits for internal_count
        // ⇽--- 2
        unsigned external_counters : 2;
        // unsigned int internal_count = 0;
        // unsigned int external_counters = 0;
    };

    struct node;

    struct counted_node_ptr
    {
        // 存在破坏trivial class 的风险
        /*bool operator == (const counted_node_ptr& cnp) {
            return (external_count == cnp.external_count && ptr == cnp.ptr);
        }*/

        // 构造初始化各成员
        // counted_node_ptr() : external_count(0), ptr(nullptr) {}
        int external_count;
        node *ptr;
    };

    struct node
    {
        std::atomic<T *> data{nullptr};
        std::atomic<node_counter> count;
        // ⇽---  1
        std::atomic<counted_node_ptr> next;

        node(int external_count = 2)
        {
            node_counter new_count;
            new_count.internal_count = 0;
            // ⇽---  4
            new_count.external_counters = external_count;
            count.store(new_count);

            counted_node_ptr node_ptr;
            node_ptr.ptr = nullptr;
            node_ptr.external_count = 0;

            next.store(node_ptr);
        }

        void release_ref()
        {
            std::cout << "call release ref " << std::endl;
            node_counter old_counter =
                count.load(std::memory_order_relaxed);
            node_counter new_counter;
            do
            {
                new_counter = old_counter;
                // 1
                --new_counter.internal_count;
            }
            // 2
            while (!count.compare_exchange_strong(
                old_counter, new_counter,
                std::memory_order_acquire, std::memory_order_relaxed));

            if (!new_counter.internal_count &&
                !new_counter.external_counters)
            {
                // 3
                delete this;
                std::cout << "release_ref delete success" << std::endl;
                destruct_count.fetch_add(1);
            }
        }
    };

    std::atomic<counted_node_ptr> head;
    // ⇽--- 1
    std::atomic<counted_node_ptr> tail;

    // ⇽---  1
    void set_new_tail(counted_node_ptr &old_tail, counted_node_ptr const &new_tail)
    {
        node *const current_tail_ptr = old_tail.ptr;
        // ⇽---  2  此处仅有一个线程能设置tail为new_tail，失败的会更新old_tail为tail的新值
        //  为防止失败的线程重试导致tail被再次更新所以添加了后面的&&判断
        // 如果tail和old_tail不等说明引用计数不同或者tail已经被移动，如果tail已经被移动那么old_tail的ptr和current_tail_ptr不同，则可以直接退出。
        // 所以一旦tail被设置为new_tail，那么另一个线程在重试时判断tail和old_tail不等，会修改old_tail, 此时old_tail已经和current_tail不一致了，所以没必要再重试。
        // 如不加后续判断， 会造成重复设置newtail，引发多插入节点的问题。
        while (!tail.compare_exchange_weak(old_tail, new_tail) && old_tail.ptr == current_tail_ptr);
        // ⇽---  3
        if (old_tail.ptr == current_tail_ptr)
        {
            // std::cout << "call free_external_counter set_new_tail" << std::endl;
            free_external_counter(old_tail); // ⇽---  4
        }
        else
        {
            current_tail_ptr->release_ref(); // ⇽---  5
        }
    }

    static void free_external_counter(counted_node_ptr &old_node_ptr)
    {
        node *const ptr = old_node_ptr.ptr;
        int const count_increase = old_node_ptr.external_count - 2;
        node_counter old_counter =
            ptr->count.load(std::memory_order_relaxed);
        node_counter new_counter;
        do
        {
            new_counter = old_counter;
            // ⇽---  1
            --new_counter.external_counters;
            // ⇽---  2
            new_counter.internal_count += count_increase;
        }
        // ⇽---  3
        while (!ptr->count.compare_exchange_strong(
            old_counter, new_counter,
            std::memory_order_acquire, std::memory_order_relaxed));

        if (!new_counter.internal_count &&
            !new_counter.external_counters)
        {
            // ⇽---  4
            destruct_count.fetch_add(1);
            std::cout << "free_external_counter delete success" << std::endl;
            delete ptr;
        }
    }

    static void increase_external_count(std::atomic<counted_node_ptr> &counter, counted_node_ptr &old_counter)
    {
        counted_node_ptr new_counter;
        do
        {
            new_counter = old_counter;
            ++new_counter.external_count;
        } while (!counter.compare_exchange_strong(old_counter, new_counter,
            std::memory_order_acquire, std::memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }

public:
    LockFreeQueue()
    {
        counted_node_ptr new_next;
        new_next.ptr = new node();
        new_next.external_count = 1;
        tail.store(new_next);
        head.store(new_next);
        // std::cout << "new_next.ptr is " << new_next.ptr << std::endl;
    }

    ~LockFreeQueue()
    {
        while (pop());
        auto head_counted_node = head.load();
        delete head_counted_node.ptr;
    }

    void push(T new_value)
    {
        std::unique_ptr<T> new_data(new T(new_value));
        counted_node_ptr new_next;
        new_next.ptr = new node;
        new_next.external_count = 1;
        counted_node_ptr old_tail = tail.load();
        for (;;)
        {
            increase_external_count(tail, old_tail);
            T *old_data = nullptr;
            // ⇽---  6
            if (old_tail.ptr->data.compare_exchange_strong(
                    old_data, new_data.get()))
            {
                // std::cout << "tail match" << std::endl;
                counted_node_ptr old_next = {0, nullptr};  // D1
                // counted_node_ptr now_next = old_tail.ptr->next.load();
                // ⇽---  7 链接新的节点, if we get the data, but block here, the update of ptr->next will be completed by other thread.
                // When we back again, we find next has changed, so we set old_next to newest next (ptr->next), we do not need the new_next 
                // in this thread, so we delete it.
                if (!old_tail.ptr->next.compare_exchange_strong(
                        old_next, new_next))
                {
                    // ⇽---  8
                    delete new_next.ptr;
                    new_next = old_next; // ⇽---  9
                }
                // std::cout << "set_new_tail in suc" << std::endl;
                set_new_tail(old_tail, new_next);
                new_data.release(); // will not delete, but give up the ownership, reset() will delete
                break;
            }
            else // ⇽---  10
            {
                counted_node_ptr old_next = {0, nullptr}; // D2
                // ⇽--- 11, if we did not get data, but get hte next, link the new next.
                if (old_tail.ptr->next.compare_exchange_strong(old_next, new_next)) // in two thread old_next may diff, but old_next->ptr is the same pointer.
                {
                    // ⇽--- 12
                    old_next = new_next;
                    // ⇽---  13
                    new_next.ptr = new node;
                }
                //  ⇽---  14
                // std::cout << "set_new_tail in fail" << std::endl;
                set_new_tail(old_tail, old_next);
            }
        }

        construct_count.fetch_add(1);
    }

    std::unique_ptr<T> pop()
    {
        counted_node_ptr old_head = head.load(std::memory_order_relaxed);
        for (;;)
        {
            increase_external_count(head, old_head);
            node *const ptr = old_head.ptr;
            if (ptr == tail.load().ptr)
            {
                // 头尾相等说明队列为空，要减少内部引用计数
                ptr->release_ref(); // D3
                return std::unique_ptr<T>();
            }
            //  ⇽---  2
            counted_node_ptr next = ptr->next.load();
            if (head.compare_exchange_strong(old_head, next))
            {
                T *const res = ptr->data.exchange(nullptr);
                // std::cout << "call free_external_counter pop" << std::endl;
                free_external_counter(old_head);
                return std::unique_ptr<T>(res);
            }
            ptr->release_ref();
        }
    }

    static std::atomic<int> destruct_count; // to check whether there is memory leak
    static std::atomic<int> construct_count;
};

template <typename T>
std::atomic<int> LockFreeQueue<T>::destruct_count = 0;

template <typename T>
std::atomic<int> LockFreeQueue<T>::construct_count = 0;