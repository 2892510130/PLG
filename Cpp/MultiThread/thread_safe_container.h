#pragma once

#include <cstdarg>
#include <cstddef>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stack>
#include <queue>
#include <utility>
#include <vector>

template<typename T>
class ThreadSafeStackWaitable
{
    private:
        std::stack<T> m_stack;
        mutable std::mutex m_mtx; // allow lock in const function
        std::condition_variable m_cv;
    
    public:
        ThreadSafeStackWaitable() {};

        ThreadSafeStackWaitable(const ThreadSafeStackWaitable &other)
        {
            std::lock_guard<std::mutex> lock(other.m_mtx);
            m_stack = other.m_stack;
        }

        ThreadSafeStackWaitable& operator=(const ThreadSafeStackWaitable&) = delete;

        void push(T value)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_stack.push(std::move(value));
            m_cv.notify_one();
        }

        void wait_and_pop(T &value)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]() {
                if (m_stack.empty()) return false;
                return true;
            });

            value = std::move(m_stack.top());
            m_stack.pop();
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]() {
                if (m_stack.empty()) return false;
                return true;
            });

            std::shared_ptr<T> const pointer(std::make_shared<T>(std::move(m_stack.top())));
            m_stack.pop();
            return pointer;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_stack.empty();
        }

        bool try_pop(T &value)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_stack.empty()) return false;
            value = std::move(m_stack.top());
            m_stack.pop();
            return true;
        }

        std::shared_ptr<T> try_pop()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_stack.empty()) return nullptr;
            std::shared_ptr<T> pointer(std::make_shared<T>(std::move(m_stack.top())));
            m_stack.pop();
            return pointer;
        }
 
};

template<typename T>
class ThreadSafeQueuePtr
{
    public:
        ThreadSafeQueuePtr() {}

        void push(T value)
        {
            std::shared_ptr<T> data(std::make_shared<T>(std::move(value)));
            std::lock_guard<std::mutex> lock(m_mtx);
            m_queue.push(data);
            m_cond.notify_one();
        }

        void wait_and_pop(T& value)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cond.wait(lock, [this]{return !m_queue.empty();});
            value = std::move(*m_queue.front()); // return a pointer, only 64 bit.
            m_queue.pop();
        }

        bool try_pop(T& value)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (m_queue.empty()) return false;

            value = std::move(*m_queue.front());
            return true;
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cond.wait(lock, [this]{return !m_queue.empty();});
            std::shared_ptr<T> res = m_queue.front();
            m_queue.pop();
            return res;
        }

        std::shared_ptr<T> try_pop()
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (m_queue.empty()) return nullptr;
            std::shared_ptr<T> res = m_queue.front();
            m_queue.pop();
            return res;
        }

        int size()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_queue.size();
        }

        void test()
        {
            std::cout << "ptr: " << m_queue.front().get() << '\n';
        }

    private:
        std::mutex m_mtx;
        std::queue<std::shared_ptr<T>> m_queue;
        std::condition_variable m_cond;
};


template<typename T>
class ThreadSafeQueueHeadTail
{
    private:
        struct node
        {
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
            node* prev; // for steal
        };

        std::unique_ptr<node> m_head;
        std::mutex m_head_mutex;
        std::mutex m_tail_mutex;
        node* m_tail;
        std::condition_variable m_cond;
        std::atomic<bool> m_bstop; // for actor_single.h

        node* get_tail()
        {
            std::lock_guard<std::mutex> lock(m_tail_mutex);
            return m_tail;
        }

        std::unique_ptr<node> pop_head()
        {
            std::unique_ptr<node> old_head = std::move(m_head);
            m_head = std::move(old_head->next);
            return old_head;
        }

        std::unique_lock<std::mutex> wait_for_data()
        {
            std::unique_lock<std::mutex> lock(m_head_mutex);
            m_cond.wait(lock, [this]() {
                return (m_bstop.load() == true) || (m_head.get() != get_tail());
            });
            // return std::move(lock);
            return lock;
        }

        std::unique_ptr<node> wait_pop_head()
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            if (m_bstop.load()) return nullptr;
            return pop_head();
        }

        std::unique_ptr<node> wait_pop_head(T& value)
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            if (m_bstop.load()) return nullptr;
            value = std::move(*m_head->data);
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head()
        {
            std::lock_guard<std::mutex> head_lock(m_head_mutex);
            if (m_head.get() == get_tail())
            {
                return std::unique_ptr<node>();
            }
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head(T& value)
        {
            std::lock_guard<std::mutex> head_lock(m_head_mutex);
            if (m_head.get() == get_tail())
            {
                return std::unique_ptr<node>();
            }
            value = std::move(*m_head->data);
            return pop_head();
        }

    public:
        ThreadSafeQueueHeadTail() : m_head(new node), m_tail(m_head.get()), m_bstop(false) {}

        ThreadSafeQueueHeadTail(const ThreadSafeQueueHeadTail&) = delete;
        ThreadSafeQueueHeadTail& operator=(const ThreadSafeQueueHeadTail&) = delete;

        void notify_stop() 
        {
            m_bstop.store(true);
            m_cond.notify_one();
        }
        
        void exit()
        {
            m_bstop.store(true);
            m_cond.notify_all();
        }

        bool wait_and_pop_timeout(T& value) {
            std::unique_lock<std::mutex> head_lock(m_head_mutex);
            auto res = m_cond.wait_for(head_lock, std::chrono::milliseconds(100),
                    [&] {return m_head.get() != get_tail() || m_bstop.load() == true; });
            if (res == false) {
                return false;
            }

            if (m_bstop.load()) {
                return false;
            }
            
            value = std::move(*m_head->data);	
            m_head = std::move(m_head->next);
            return true;
        }

        std::shared_ptr<T> wait_and_pop() //  <------3
        {
            std::unique_ptr<node> const old_head = wait_pop_head();
            if (old_head == nullptr) return nullptr;
            return old_head->data;
        }

        bool wait_and_pop(T& value)  //  <------4
        {
            std::unique_ptr<node> const old_head = wait_pop_head(value);
            if (old_head == nullptr) return false;
            return true;
        }

        std::shared_ptr<T> try_pop()
        {
            std::unique_ptr<node> old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        bool try_pop(T& value)
        {
            std::unique_ptr<node> const old_head = try_pop_head(value);
            return old_head ? true : false;
        }

        bool empty()
        {
            std::lock_guard<std::mutex> head_lock(m_head_mutex);
            return (m_head.get() == get_tail());
        }

        void push(T new_value)  //<------2
        {
            std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
            std::unique_ptr<node> p(new node);
            {
                std::lock_guard<std::mutex> tail_lock(m_tail_mutex);
                node* const new_tail = p.get();
                new_tail->prev = m_tail;
                m_tail->data = new_data;
                m_tail->next = std::move(p);
                m_tail = new_tail;
            }
            m_cond.notify_one();
        }

        bool try_steal(T& value) // pop from tail
        {
            std::unique_lock<std::mutex> tail_lock(m_tail_mutex,std::defer_lock);
            std::unique_lock<std::mutex> head_lock(m_head_mutex, std::defer_lock);
            std::lock(tail_lock, head_lock);
            if (m_head.get() == m_tail)
            {
                return false;
            }

            node* prev_node = tail->prev;
            value = std::move(*(prev_node->data));
            m_tail = prev_node;
            m_tail->next = nullptr;
            return true;
        }
};