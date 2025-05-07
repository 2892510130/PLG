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

        template <class F, class... Args>
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
        ThreadPool(unsigned int num = 5) : m_stop(false)
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
