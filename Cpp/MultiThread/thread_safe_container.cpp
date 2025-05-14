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

        std::shared_ptr<T> pop()
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
        };

        std::unique_ptr<node> m_head;
        std::mutex m_head_mutex;
        std::mutex m_tail_mutex;
        node* m_tail;
        std::condition_variable m_cond;

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
                return m_head.get() != get_tail();
            });
            return std::move(lock);
        }

        std::unique_ptr<node> wait_pop_head()
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());   
            return pop_head();
        }

        std::unique_ptr<node> wait_pop_head(T& value)
        {
            std::unique_lock<std::mutex> head_lock(wait_for_data());  
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
        ThreadSafeQueueHeadTail() : m_head(new node), m_tail(new node) {}

        ThreadSafeQueueHeadTail(const ThreadSafeQueueHeadTail&) = delete;
        ThreadSafeQueueHeadTail& operator=(const ThreadSafeQueueHeadTail&) = delete;

        std::shared_ptr<T> wait_and_pop() //  <------3
        {
            std::unique_ptr<node> const old_head = wait_pop_head();
            return old_head->data;
        }

        void wait_and_pop(T& value)  //  <------4
        {
            std::unique_ptr<node> const old_head = wait_pop_head(value);
        }


        std::shared_ptr<T> try_pop()
        {
            std::unique_ptr<node> old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        bool try_pop(T& value)
        {
            std::unique_ptr<node> const old_head = try_pop_head(value);
            return old_head;
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
            node* const new_tail = p.get();
            std::lock_guard<std::mutex> tail_lock(m_tail_mutex);
            m_tail->data = new_data;
            m_tail->next = std::move(p);
            m_tail = new_tail;
        }
};

void test_safe_queue()
{
    ThreadSafeQueuePtr<std::vector<int>> tsqp;
    std::vector<int> data{1, 2, 3, 4, 5};
    tsqp.push(data);
    tsqp.test();
    std::shared_ptr<std::vector<int>> res = tsqp.try_pop();
    std::cout << "data: " << (*res)[1] << '\n';
    std::cout << "ptr: " << res << '\n';
}

int main()
{
    /*
     *  Thread safe stack and queue:
     *  1. in mutex_loc.cpp we implement a thread safe queue with mutex and throw exception,
     *     we now upgrade it.
     *     1.1 we should not throw exception when stack is empty (as it's a common situation). 
     *     So we use condition_variable and impl two pop method, one wait, one doesn't.
     *     1.2 only one problem : first for vector like data, we may get exception when we move the data (memory
     *     not enough, even though we use move, some condition like user defined allocator, we
     *     may re-allocate), now this thread do not notify_one, ohter thread can not wake, 
     *     and stack top is moved. We can fix it by using stack<shared_ptr<T>>, see the safe queue.
     *  2. in safe_queue.cpp we impl a thread safe queue, new we upgrade it with save shared_ptr.
     *     when pop we return a pointer, so the memory will be enough (most most of the time);
     *     2.1 However the push and pop are sequential, we need to seperate them. We can use a 
     *     cycle queue like in no_lock_queue, but here we impl another way (using a virtual point).
     *     tail always point to this virtual node, and head node move forward and point to the next;
     * */
    test_safe_queue();
}

