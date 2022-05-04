
#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <functional>
#include <future>

#include "Synapse/Core.hpp"
#include "ThreadSafeQueue.hpp"


namespace Syn
{
    //
    class ThreadPool
    {
        
    private:
        class ThreadWorker
        {
        private:
            ThreadPool* m_pool;
            int m_id;
        public:
            ThreadWorker(ThreadPool* _pool, const int _id) : 
                m_pool(_pool), m_id(_id) {}

            void operator()()
            {
                std::function<void()> func;
                bool assigned_task;
                while (!m_pool->m_done)
                {
                    {
                        std::unique_lock<std::mutex> lock(m_pool->m_mutex);
                        // if queue is empty, wait for work (i.e. to be notify_one():ed)
                        if (m_pool->m_queue.empty())
                        {
                            #ifdef DEBUG_THREADPOOL
                                SYN_CORE_TRACE("thread " << m_id << " waiting for work");
                            #endif
                            m_pool->m_conditionalLock.wait(lock);
                        }
                        // else grab something to do
                        assigned_task = m_pool->m_queue.pop(func);
                    }
                    if (assigned_task)
                    {
                        #ifdef DEBUG_THREADPOOL
                            SYN_CORE_TRACE("thread " << m_id << " starting task");
                        #endif
                        func();
                    }
                }
                #ifdef DEBUG_THREADPOOL
                    SYN_CORE_TRACE("thread " << m_id << " shutting down");
                #endif
            }
        };

        ThreadSafeQueue<std::function<void()>> m_queue;
        std::vector<std::thread> m_threads;
        bool m_done = false;
        std::mutex m_mutex;
        std::condition_variable m_conditionalLock;

    public:
        // use the maximum number of threads, saving one for the main thread
        ThreadPool(const int _n_threads = std::thread::hardware_concurrency()-1) :
            m_threads(std::vector<std::thread>(_n_threads)), m_done(false) 
        {}

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        ~ThreadPool() {}
        
        const size_t threadCount() { return m_threads.size(); }
        static ThreadPool& get()
        {
            static ThreadPool instance;
            return instance;
        }

        //
        void init()
        {
            SYN_CORE_TRACE("initializing worker threads (", m_threads.size(),").");

            #ifdef DEBUG_THREADPOOL
                SYN_CORE_TRACE("creating " << m_threads.size() << " worker threads");
            #endif
            for (size_t i = 0; i < m_threads.size(); i++)
                m_threads[i] = std::thread(ThreadWorker(this, i));
        }

        //
        void shutdown()
        {
            m_done = true;
            m_conditionalLock.notify_all();
            #ifdef DEBUG_THREADPOOL
                SYN_CORE_TRACE("releasing worker threads");
            #endif
            for (size_t i = 0; i < m_threads.size(); i++)
                if (m_threads[i].joinable())
                    m_threads[i].join();

            SYN_CORE_TRACE("worker threads destroyed.");
        }

        // Submit a task to be executed by a worker thread.
        template<typename F, typename... Args>
        auto submit(F&& _func, Args&&... _args) 
            -> std::future<decltype(_func(_args...))>
        {
            // create a function with arguments _args and return type of decltype(_func(_args))
            std::function<decltype(_func(_args...))()> func = std::bind(std::forward<F>(_func), 
                                                                        std::forward<Args>(_args)...);
            // Wrap (using a smart pointer) to a void function(void), to be inserted into the queue.
            // We use a smart pointer so that the function call is still valid when the pointer
            // goes out of scope (in the current function).
            auto task_ptr = std::make_shared<std::packaged_task<decltype(_func(_args...))()>>(func);
            std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };
            // put into queue
            m_queue.push(wrapper_func);
            // wake one thread
            m_conditionalLock.notify_one();
            // return a future from promise
            return task_ptr->get_future();
        }

    };

}

