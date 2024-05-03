/**
 * @file blockQueue.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 阻塞队列
 * @version 0.1
 * @date 2024-05-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <cassert>
#include <mutex>
#ifndef _BLOCK_QUEUE_H
#define _BLOCK_QUEUE_H
#include<deque>
#include <condition_variable>
/**
 * @brief 阻塞队列
 * 
 * @tparam T 
 */
template<typename T>
class BlockQueue{
    public:
        explicit BlockQueue(size_t MaxCapacity);
        ~BlockQueue();
        void clear();
        bool empty() ;
        bool full() ;
        void Close();
        size_t size() ;
        size_t capacity() ;
        T front();
        T back();
        void push_back(const T& item);
        void push_front(const T& item);
        bool pop(T &item);
        bool pop(T &item,int timeout);
        void flush();
    private:
        std::deque<T> deq;//阻塞队列里面的队列
        size_t capacity_;//容量
        std::mutex mtx_;//互斥锁
        bool isClose_;//是否关闭阻塞队列
        std::condition_variable condConsumer;//消费者条件变量
        std::condition_variable condProducer;//生产者条件变量
};
#endif

template <typename T> inline BlockQueue<T>::BlockQueue(size_t MaxCapacity):capacity_(MaxCapacity) {
    assert(MaxCapacity>0);//断言最大容量大于0
    isClose_ = false;
}

template <typename T> inline BlockQueue<T>::~BlockQueue() {
    Close();
}

template <typename T> inline void BlockQueue<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deq.clear();
}

template <typename T> inline bool BlockQueue<T>::empty() { 
    std::lock_guard<std::mutex> locker(mtx_);
    return deq.empty();
 }

template <typename T> inline bool BlockQueue<T>::full()  { 
    std::lock_guard<std::mutex> locker(mtx_);
    return deq.size() == capacity_;
 }

template <typename T> void BlockQueue<T>::Close() {
  std::lock_guard<std::mutex> locker(mtx_);
  deq.clear(); // 清空队列
  isClose_ = true;
  // 唤醒所有的消费者和生产者
  condProducer.notify_all();
  condConsumer.notify_all();
}

template <typename T> inline size_t BlockQueue<T>::size() { 
    std::lock_guard<std::mutex> locker(mtx_);
    return deq.size();
 }

template <typename T> inline size_t BlockQueue<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
 }

template <typename T> inline T BlockQueue<T>::front() { 
    std::lock_guard<std::mutex> locker(mtx_);
    return deq.front();
 }

 template <typename T> inline T BlockQueue<T>::back() { 
    std::lock_guard<std::mutex> locker(mtx_);
    return deq.back();
  }

 template <typename T> inline void BlockQueue<T>::flush() {
   condConsumer.notify_one(); // 唤醒等待的消费者
 }  
 template<typename T> 
 void BlockQueue<T>::push_back(const T&item){
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq.size()>=capacity_){//当队列到达的最高容量时应该等待有空位
        condProducer.wait(locker);//等待生产者条件变量
    }
    deq.push_back(item);
    condConsumer.notify_one();
 }
 template<typename T> 
 void BlockQueue<T>::push_front(const T&item){
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq.size()>=capacity_){//当队列到达的最高容量时应该等待有空位
        condProducer.wait(locker);//等待生产者条件变量
    }
    deq.push_front(item);
    condConsumer.notify_one();
 }
 template<typename T> 
 bool BlockQueue<T>::pop( T&item){
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq.empty())//队列为空就要等待消费者
    {
        condConsumer.wait(locker);
        if(isClose_){//如果队列关闭就直接退出不操作了
            return false;
        }
    }
    item = deq.front();
    deq.pop_front();
    condProducer.notify_one();//通知消费者
    return true;
 }
 template<typename T> 
 bool BlockQueue<T>::pop( T&item,int timeout){
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq.empty())//队列为空就要等待消费者
    {
        condConsumer.wait_for(locker,std::chrono::seconds(timeout));
        if(isClose_){//如果队列关闭就直接退出不操作了
            return false;
        }
    }
    item = deq.front();
    deq.pop_front();
    condProducer.notify_one();//通知消费者
    return true;
 }