// #pragma once
// #include <iostream>
// #include <stdexcept>
// #include <vector>
// #include <map>
// #include <set>
// #include <algorithm>
// #include <mutex>
// #include <deque>
// #include <condition_variable>
// #include <thread>
// #include <pthread.h>
// #include <unistd.h>

// template <typename T>
// class SharedQueue {
// public:
//     SharedQueue();
//     ~SharedQueue();
//     bool pop(std::unique_ptr<T>&& pData);
//     void push(std::unique_ptr<T>&& item);
//     int size();
//     bool empty();

// public:
//     T operator[](int k)
//     {
//         return queue_[k];
//     }

// private:
//     std::deque<std::unique_ptr<T>> queue_;
//     std::mutex mutex_;
//     std::condition_variable cond_;
// };

// template <typename T>
// SharedQueue<T>::SharedQueue() {}

// template <typename T>
// SharedQueue<T>::~SharedQueue() {}

// template <typename T>
// bool SharedQueue<T>::pop(std::unique_ptr<T>&& pData)
// {
//     std::unique_lock<std::mutex> mlock(mutex_);
//     if (queue_.empty()) {
//         return false;
//     }
//     pData = std::move(queue_.front());
//     queue_.pop_front();
//     return true;
// }

// template <typename T>
// void SharedQueue<T>::push(std::unique_ptr<T>&& item)
// {
//     std::unique_lock<std::mutex> mlock(mutex_);
//     queue_.push_back(std::move(item));
//     mlock.unlock();     // unlock before notificiation to minimize mutex con
//     cond_.notify_all(); // notify all waiting threads
// }

// template <typename T>
// int SharedQueue<T>::size()
// {
//     std::unique_lock<std::mutex> mlock(mutex_);
//     int size = queue_.size();
//     mlock.unlock();
//     return size;
// }

// template <typename T>
// bool SharedQueue<T>::empty()
// {
//     std::unique_lock<std::mutex> mlock(mutex_);
//     return queue_.empty();
// }

// SharedQueue<int> sq;
// std::mutex mtx;
// void f(int idx)
// {
//     mtx.lock();
//     std::cout << "threadIdx: " << idx << "\n";
//     mtx.unlock();
//     std::unique_ptr<int> pData;
//     while(sq.pop(std::move(pData))) {
//         mtx.lock();
//         std::cout << *pData << "\n";
//         mtx.unlock();
//         pData.reset();
//     }
// }

// int main() {
//     for (int i = 0; i < 10; i++) {
//         std::unique_ptr<int> pData = std::make_unique<int>(i);
//         sq.push(std::move(pData));
//     }
//     std::vector<std::thread> ths;
//     int thNum = 4;
//     for (int i = 0; i < thNum; i++) {
//         std::thread th(f, i);
//         ths.push_back(std::move(th));
//     }
//     for (int i = 0; i < thNum; i++) {
//         ths[i].join();
//     }
//     return 0;
// }

#ifndef __CONCURRENCEQUEUE_H__
#define __CONCURRENCEQUEUE_H__
#include <mutex>
#include <condition_variable>
#include <deque>
#include <queue>
#include <memory>
#include <thread>

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
    
    void push(const DATATYPE & data) {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_data.push(data);
        m_cond.notify_one();
    }
    
    void push(DATATYPE && data) {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_data.push(std::move(data));
        m_cond.notify_one();
    }
    
    std::shared_ptr<DATATYPE> tryPop() {  // 非阻塞
        std::lock_guard<std::mutex> lg(m_mutex);
        if (m_data.empty()) return {};
        auto res = std::make_shared<DATATYPE>(m_data.front());
        m_data.pop();
        return res;
    }
    
    std::shared_ptr<DATATYPE> pop() {  // 非阻塞
        std::unique_lock<std::mutex> lg(m_mutex);
        m_cond.wait(lg, [this] { return !m_data.empty(); });
        auto res = std::make_shared<DATATYPE>(std::move(m_data.front()));
        m_data.pop();
        return res;
    }
    
private:
    std::queue<DATATYPE, SEQUENCE> m_data;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};
#endif

class ClusterTask
{
    public:
    int aaa;
    ClusterTask(int aaa_)
    {
        aaa = aaa;
    }

    void aaaa()
    {
         std::printf("[1aaaa]  -------   %d\n", aaa);
    }
};

ConcurrenceQueue<ClusterTask> g_queue;

void producer() {    
    for (int i = 0; i < 100; ++i) {
        ClusterTask a(i);
        g_queue.push(a);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}
void consumer1() {
    while (1) {
        // std::printf("[1]  -------   %d\n", g_queue.pop()->aaaa());
        g_queue.pop()->aaaa();
        std::printf("aaaaaaaaaaaaa \n");
    }
}
void consumer2() {
    while (1) {
        auto front = g_queue.tryPop();
        front->aaaa();
        std::printf("[2]  -------   %d\n", front ? *front : -1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main () {
    std::thread t1(producer);
    std::thread t2(consumer1);

    t1.join();
    t2.detach();
    return 0;
}

// #include <iostream>
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <queue>

// #include <memory>

// class ClusterTask
// {
//     public:
//     int aaa;
//     ClusterTask(int aaa_)
//     {
//         aaa = aaa;
//     }

//     void aaaa()
//     {
//          std::printf("[1aaaa]  -------   %d\n", aaa);
//     }
// };


// template<typename T>

// class threadsafe_queue {
// private:
//     mutable std::mutex mtx;     // 互斥必须用mutable修饰（针对const对象，准许其数据成员发生变动）
//     std::queue<T> data_queue;
//     std::condition_variable data_cond;
// public:
//     threadsafe_queue() {}

//     threadsafe_queue(threadsafe_queue const& other) {
//         std::lock_guard<std::mutex> lk(other.mtx);
//         data_queue = other.data_queue;
//     }

//     threadsafe_queue(threadsafe_queue&& other) {
//         std::lock_guard<std::mutex> lk(other.mtx);
//         data_queue = std::move(other.data_queue);
//     }

//     threadsafe_queue& operator=(threadsafe_queue const& other) {
//         std::lock_guard<std::mutex> lk(mtx);
//         data_queue = other.data_queue;
//         return *this;
//     }

//     threadsafe_queue& operator=(threadsafe_queue&& other) {
//         std::lock_guard<std::mutex> lk(mtx);
//         data_queue = std::move(other.data_queue);
//         return *this;
//     }
    
//     void push(T new_value) {
//         std::lock_guard<std::mutex> lk(mtx);
//         data_queue.push(new_value);
//         data_cond.notify_one();
//     }

//     void wait_and_pop(T& value) {
//         std::unique_lock<std::mutex> lk(mtx);
//         data_cond.wait(lk,[this]{
//             return !data_queue.empty();
//         });
//         value = data_queue.front();
//         data_queue.pop();
//     }

//     std::shared_ptr<T> wait_and_pop() {
//         std::unique_lock<std::mutex> lk(mtx);
//         data_cond.wait(lk, [this] {
//            return !data_queue.empty();
//         });
//         std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
//         data_queue.pop();
//         return res;
//     }

//     bool try_pop(T& value) {
//         std::lock_guard<std::mutex> lk(mtx);
//         if (data_queue.empty()) {
//             return false;
//         }
//         value = data_queue.front();
//         data_queue.pop();
//         return true;
//     }

//     std::shared_ptr<T> try_pop() {
//         std::lock_guard<std::mutex> lk(mtx);
//         if (data_queue.empty()) {
//             return std::shared_ptr<T>();
//         }
//         std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
//         data_queue.pop();
//         return res;
//     }

//     bool empty() const {
//         std::lock_guard<std::mutex> lk(mtx);
//         return data_queue.empty();
//     }

//     size_t size() const {
//         std::lock_guard<std::mutex> lk(mtx);
//         return data_queue.size();
//     }
// };


// void producer(threadsafe_queue<ClusterTask>& q) {
//     for (int i = 0; i < 10; ++i) {
//         ClusterTask a(i);
//         q.push(a);
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
// }

// void consumer(threadsafe_queue<ClusterTask>& q) {
//     // int value;
//     ClusterTask value(0);
//     for (int i = 0; i < 10; ++i) {
//         q.wait_and_pop(value);
//         value.aaaa();
//         // std::cout << "Consumer got value: " << value.aaaa() << std::endl;
//     }
// }

// int main() {
//     threadsafe_queue<ClusterTask> q;
//     std::thread t1(producer, std::ref(q));
//     std::thread t2(consumer, std::ref(q));
//     t1.join();
//     t2.join();
//     return 0;
// }
