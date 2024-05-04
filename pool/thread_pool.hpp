/**
 * @file thread_pool.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 
 * @version 0.1
 * @date 2024-05-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_
#include <mutex>
#include <condition_variable>
#include <queue>
#include <assert.h>
#include <functional>
#include <thread>
/**
* @brief 一个基本的处理队列池
* 
*/
struct Pool{
    std::mutex mtx;
    std::condition_variable cond;
    bool isClosed;
    std::queue<std::function<void()>> tasks;
};
class ThreadPool{
    public:
        explicit ThreadPool(size_t thread_count = 8):pool_(std::shared_ptr<Pool>()){
            assert(thread_count>0);//断言线程池的线程数目大于0
            for(int i = 0; i < thread_count;i++){
                std::thread([pool = pool_]{//创建线程池工作线程
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while (true)
                    {
                        if(!pool->tasks.empty())
                        {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        }else if(pool->isClosed) break;
                        else pool->cond.wait(locker);
                    }
                }).detach();                
            }
        }
        ThreadPool() = default;
        ~ThreadPool(){
            if(static_cast<bool>(pool_)){
                {
                    std::lock_guard<std::mutex> locker(pool_->mtx);
                    pool_->isClosed = true;
                }
                pool_->cond.notify_all();
            }
        }
        template<typename T>
        void AddTasK(T &&task){
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->tasks.emplace(std::forward<T>(task));
            }
            pool_->cond.notify_one();
        }
    private:
        
        std::shared_ptr<Pool> pool_;
};
#endif