#pragma once

#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <future>
#include <iostream>
#include <condition_variable>
#include <functional>

#include "thread_safe_container.h"
#include "join_thread.h"

class ThreadPool
{
    using Task = std::packaged_task<void()>;

    public:
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        static ThreadPool& get_instance(unsigned int thread_num = 5)
        {
            static ThreadPool ins(thread_num);
            return ins;
        }

        ~ThreadPool() { stop(); }

        template <class F, class... Args> // or decltype(f(std::forward<Args>(args)...)), if args has right value
        auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
            using RetType = decltype(f(args...));
            if (m_stop.load())
                return std::future<RetType>{};

            auto task = std::make_shared<std::packaged_task<RetType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<RetType> ret = task->get_future();
            {
                std::lock_guard<std::mutex> cv_mt(m_cv_mtx);
                m_tasks.emplace([task] { (*task)(); });
            }
            m_cond_lock.notify_one();
            return ret;
        }

        int idleThreadCount() {
            return m_thread_num;
        }
    
    private:
        ThreadPool(unsigned int num = std::thread::hardware_concurrency()) : m_stop(false)
        {
            if (num < 1)
                m_thread_num = 1;
            else
                m_thread_num = num;
            
            std::cout << "Start thread pool with " << m_thread_num << " threads\n";
            start();
        }

        void start()
        {
            for (int i = 0; i < m_thread_num; ++i) {
                m_pool.emplace_back([this]() {
                    while (!this->m_stop.load()) {
                        Task task;
                        {
                            std::unique_lock<std::mutex> cv_mt(m_cv_mtx);
                            this->m_cond_lock.wait(cv_mt, [this] {
                                return this->m_stop.load() || !this->m_tasks.empty();
                            });
                            if (this->m_tasks.empty()) // when m_stop is true and we need to finish the task then kill the thread
                                return;
    
                            task = std::move(this->m_tasks.front());
                            this->m_tasks.pop();
                        }
                        this->m_thread_num--;
                        task();
                        this->m_thread_num++;
                    }
                });
            }
        }

        void stop()
        {
            m_stop.store(true);
            m_cond_lock.notify_all();
            for (auto& td : m_pool) {
                if (td.joinable()) {
                    std::cout << "join thread " << td.get_id() << std::endl;
                    td.join();
                }
            }
        }

    private:
        std::mutex m_cv_mtx;
        std::condition_variable m_cond_lock;
        std::atomic_bool m_stop;
        std::atomic_int m_thread_num;
        std::queue<Task> m_tasks;
        std::vector<std::thread> m_pool;
};

/*
    The code below is from book 'C++ concurrency in action'.
*/

// 1. simple thread pool
class simple_thread_pool
{
    std::atomic_bool done;
    //⇽-- - 1
    ThreadSafeQueueHeadTail<std::function<void()>> work_queue; 
    //⇽-- - 2
    std::vector<std::thread> threads; 
    //⇽-- - 3
    JoinThreads joiner;

    void worker_thread()
    {
        //⇽-- - 4
        while (!done)    
        {
            std::function<void()> task;
            //⇽-- - 5
            if (work_queue.try_pop(task))    
            {
                //⇽-- - 6
                task();    
            }
            else
            {
                //⇽-- - 7
                std::this_thread::yield();    
            }
        }
    }

    simple_thread_pool() : done(false), joiner(threads)
    {
        //⇽--- 8
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                //⇽-- - 9, if we do not use lambda [this]() {}, then we must pass this to make the compiler know
                threads.push_back(std::thread(&simple_thread_pool::worker_thread, this)); // which inst will do this
            }
        }
        catch (...)
        {
            //⇽-- - 10
            done = true;
            throw;
        }
    }
public:
    static simple_thread_pool& instance() {
       static simple_thread_pool pool;
       return pool;
    }

    ~simple_thread_pool()
    {
        //⇽-- - 11
        done = true;     
        // for (unsigned i = 0; i < threads.size(); ++i) // why join? JoinThreads should handle this
        // {
        //     //⇽-- - 9
        //     threads[i].join();
        // }
    }

    template<typename FunctionType>
    void submit(FunctionType f)
    {
        //⇽-- - 12
        work_queue.push(std::function<void()>(f));    
    }
};

class function_wrapper
{
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };
    std::unique_ptr<impl_base> impl;

    template<typename F>
    struct impl_type : impl_base
    {
        F f;
        impl_type(F&& f_) : f(std::move(f_)) {}
        void call() { f(); }
    };
public:
    template<typename F>
    function_wrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}
    void operator()() { impl->call(); }
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) : impl(std::move(other.impl)) {}

    function_wrapper& operator=(function_wrapper&& other)
    {
        impl = std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};

// 2. thread pool can get task status
class future_thread_pool
{
private:
    void worker_thread()
    {
        while (!done)
        {
            function_wrapper task;    
            if (work_queue.try_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
public:

    static future_thread_pool& instance() {
        static future_thread_pool pool;
        return pool;
    }

    ~future_thread_pool()
    {
        //⇽-- - 11
        done = true;
        // for (unsigned i = 0; i < threads.size(); ++i) // why join? JoinThreads should handle this
        // {
        //     //⇽-- - 9
        //     threads[i].join();
        // }
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;   
        std::packaged_task<result_type()> task(std::move(f));   
        std::future<result_type> res(task.get_future());    
        work_queue.push(std::move(task));    
        return res;   
    }

private:
    future_thread_pool() :
        done(false), joiner(threads)
    {
        //⇽--- 8
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                //⇽-- - 9
                threads.push_back(std::thread(&future_thread_pool::worker_thread, this));
            }
        }
        catch (...)
        {
            //⇽-- - 10
            done = true;
            throw;
        }
    }

    std::atomic_bool done;
    //⇽-- - 1
    ThreadSafeQueueHeadTail<function_wrapper> work_queue;
    //⇽-- - 2
    std::vector<std::thread> threads;
    //⇽-- - 3
    JoinThreads joiner;
};

// 3. wait and notify pool, it no tasks, wait rather than pool it over and over again
// use wait_and_pop rather than try_pop
class notify_thread_pool
{
private:
    void worker_thread()
    {
        while (!done)
        {
            
            auto task_ptr = work_queue.wait_and_pop();
            if (task_ptr == nullptr) {
                continue;
            }

            (*task_ptr)();
        }
    }
public:

    static notify_thread_pool& instance() {
        static  notify_thread_pool pool;
        return pool;
    }

    ~notify_thread_pool()
    {
        //⇽-- - 11
        done = true;
        work_queue.exit();
        // for (unsigned i = 0; i < threads.size(); ++i) // really needed if we have joiner?
        // {
        //     //⇽-- - 9
        //     threads[i].join();
        // }
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;   
            std::packaged_task<result_type()> task(std::move(f));   
            std::future<result_type> res(task.get_future());    
            work_queue.push(std::move(task));    
            return res;   
    }

private:
    notify_thread_pool() : done(false), joiner(threads)
    {
        //⇽--- 8
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                //⇽-- - 9
                threads.push_back(std::thread(&notify_thread_pool::worker_thread, this));
            }
        }
        catch (...)
        {
            //⇽-- - 10
            done = true;
            work_queue.exit();
            throw;
        }
    }

    std::atomic_bool done;
    //⇽-- - 1
    ThreadSafeQueueHeadTail<function_wrapper> work_queue;
    //⇽-- - 2
    std::vector<std::thread> threads;
    //⇽-- - 3
    JoinThreads joiner;
};

// 4. parrallel thread pool, create multiplu queue, so if there are too many task submit, we can handle it.
// Each thread has it's own queue
class parrallen_thread_pool
{
private:

    void worker_thread(int index)
    {
        while (!done)
        {

            auto task_ptr = thread_work_ques[index].wait_and_pop();
            if (task_ptr == nullptr) {
                continue;
            }

            (*task_ptr)();
        }
    }
public:

    static parrallen_thread_pool& instance() {
        static  parrallen_thread_pool pool;
        return pool;
    }

    ~parrallen_thread_pool()
    {
        //⇽-- - 11
        done = true;
        for (unsigned i = 0; i < thread_work_ques.size(); i++) {
            thread_work_ques[i].exit();
        }
        
        // for (unsigned i = 0; i < threads.size(); ++i) // again the same as above
        // {
        //     //⇽-- - 9
        //     threads[i].join();
        // }
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f)
    {
        int index = (atm_index.load() + 1) % thread_work_ques.size();
        atm_index.store(index);
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        thread_work_ques[index].push(std::move(task));
        return res;
    }

private:
    parrallen_thread_pool() : done(false), joiner(threads), atm_index(0)
    {
        //⇽--- 8
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            thread_work_ques = std::vector<ThreadSafeQueueHeadTail<function_wrapper>>(thread_count);

            for (unsigned i = 0; i < thread_count; ++i)
            {
                //⇽-- - 9
                threads.push_back(std::thread(&parrallen_thread_pool::worker_thread, this, i));
            }
        }
        catch (...)
        {
            //⇽-- - 10
            done = true;
            for (int i = 0; i < thread_work_ques.size(); i++) {
                thread_work_ques[i].exit();
            }
            throw;
        }
    }

    std::atomic_bool done;
    //全局队列
    std::vector<ThreadSafeQueueHeadTail<function_wrapper>> thread_work_ques;

    //⇽-- - 2
    std::vector<std::thread> threads;
    //⇽-- - 3
    JoinThreads joiner;
    std::atomic<int> atm_index;
};

// 5. above each thread has it's own thread, so maybe some thread will accmulate too many tasks,
// so other thread should help it, steal some task from other thread's queue.
// basically loop other thread queue, and try_pop them
class steal_thread_pool
{
private:

    void worker_thread(int index)
    {
        while (!done)
        {
            function_wrapper wrapper;
            bool pop_res = thread_work_ques[index].try_pop(wrapper);
            if (pop_res) {
                wrapper();
                continue;
            }

            bool steal_res = false;
            for (int i = 0; i < thread_work_ques.size(); i++) {
                if (i == index) {
                    continue;
                }

                steal_res  = thread_work_ques[i].try_steal(wrapper);
                if (steal_res) {
                    wrapper();
                    break;
                }

            }

            if (steal_res) {
                continue;
            }
            
            std::this_thread::yield();
        }
    }
public:

    static steal_thread_pool& instance() {
        static  steal_thread_pool pool;
        return pool;
    }
    ~steal_thread_pool()
    {
        //⇽-- - 11
        done = true;
        for (unsigned i = 0; i < thread_work_ques.size(); i++) {
            thread_work_ques[i].exit();
        }

        for (unsigned i = 0; i < threads.size(); ++i)
        {
            //⇽-- - 9
            threads[i].join();
        }
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type>
        submit(FunctionType f)
    {
        int index = (atm_index.load() + 1) % thread_work_ques.size();
        atm_index.store(index);
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        thread_work_ques[index].push(std::move(task));
        return res;
    }

private:
    steal_thread_pool() :
        done(false), joiner(threads), atm_index(0)
    {
        //⇽--- 8
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            thread_work_ques = std::vector < ThreadSafeQueueHeadTail<function_wrapper>>(thread_count);

            for (unsigned i = 0; i < thread_count; ++i)
            {
                //⇽-- - 9
                threads.push_back(std::thread(&steal_thread_pool::worker_thread, this, i));
            }
        }
        catch (...)
        {
            //⇽-- - 10
            done = true;
            for (int i = 0; i < thread_work_ques.size(); i++) {
                thread_work_ques[i].exit();
            }
            throw;
        }
    }

    std::atomic_bool done;
    //全局队列
    std::vector<ThreadSafeQueueHeadTail<function_wrapper>> thread_work_ques;

    //⇽-- - 2
    std::vector<std::thread> threads;
    //⇽-- - 3
    JoinThreads joiner;
    std::atomic<int>  atm_index;
};