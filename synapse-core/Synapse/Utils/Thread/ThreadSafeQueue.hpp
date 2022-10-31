
#pragma once

#include <queue>

namespace Syn
{
    template<typename T>
    class ThreadSafeQueue
    {
    private:
        std::queue<T> m_queue;
        mutable std::mutex m_mutex;

    public:
        ThreadSafeQueue() {};
        ThreadSafeQueue(const ThreadSafeQueue<T>&) = delete;
        ThreadSafeQueue(ThreadSafeQueue<T>&& _other)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue = std::move(_other.m_queue);
        }

        bool empty() const 
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }

        size_t size() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.size();
        }

        bool pop(T& _t)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty())
                return false;
            _t = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }

        void push(const T& _t)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(_t);
        }
    };

}
