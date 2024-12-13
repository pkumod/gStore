#pragma once
#include <mutex>
#include <condition_variable>
#include <deque>
#include <queue>
#include <memory>
#include <thread>

namespace cluster
{
    template<typename DATATYPE, typename SEQUENCE = std::deque<DATATYPE>>
    class ConcurrenceQueue {
    public:
        ConcurrenceQueue() = default;
        
        ConcurrenceQueue(const ConcurrenceQueue & other) {
            std::lock_guard<std::mutex> lg(other.m_mutex);
            m_data = other.m_data;
        }
        ConcurrenceQueue(ConcurrenceQueue &&) = delete;
        ConcurrenceQueue & operator= (const ConcurrenceQueue &) = delete;
        ~ConcurrenceQueue() = default;
        bool empty() const {
            std::lock_guard<std::mutex> lg(m_mutex);
            return m_data.empty();
        }
        
        // void push(const DATATYPE & data) {
        //     std::lock_guard<std::mutex> lg(m_mutex);
        //     m_data.push(data);
        //     m_cond.notify_one();
        // }
        
        // void push(DATATYPE && data) {
        //     std::lock_guard<std::mutex> lg(m_mutex);
        //     m_data.push(std::move(data));
        //     m_cond.notify_one();
        // }
        
        // std::shared_ptr<DATATYPE> tryPop() {  // 非阻塞
        //     std::lock_guard<std::mutex> lg(m_mutex);
        //     if (m_data.empty()) return {};
        //     auto res = std::make_shared<DATATYPE>(m_data.front());
        //     m_data.pop();
        //     return res;
        // }
        
        // std::shared_ptr<DATATYPE> pop() {  // 非阻塞
        //     std::unique_lock<std::mutex> lg(m_mutex);
        //     m_cond.wait(lg, [this] { return !m_data.empty(); });
        //     auto res = std::make_shared<DATATYPE>(std::move(m_data.front()));
        //     m_data.pop();
        //     return res;
        // }

        void push(DATATYPE & data)
        {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_data.push(data);
            m_cond.notify_one();
        }
        
        DATATYPE pop()
        {  // 非阻塞
            std::unique_lock<std::mutex> lg(m_mutex);
            m_cond.wait(lg, [this] { return !m_data.empty(); });
            auto res = m_data.front();
            m_data.pop();
            return res;
        }

        void clear()
        {
            std::unique_lock<std::mutex> lg(m_mutex);
            while (!m_data.empty())
                m_data.pop();   
        }
        
    private:
        std::queue<DATATYPE, SEQUENCE> m_data;
        mutable std::mutex m_mutex;
        std::condition_variable m_cond;
    };
}