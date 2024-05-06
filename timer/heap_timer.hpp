/**
 * @file heap_timer.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 
 * @version 0.1
 * @date 2024-05-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _HEAP_TIMER_H_
#define _HEAP_TIMER_H_
#include <cstddef>
#include <functional>
#include <chrono>
#include <vector>
#include <unordered_map>
typedef std::function<void()> timeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::microseconds Ms; 
typedef Clock::time_point TimeStamp;
typedef struct TimerNode {
  int id;
  TimeStamp expires;
  timeoutCallBack call_back;
  bool operator<(const TimerNode &other) const {
    return expires < other.expires;
  }
} timer;
class HeapTimer{
    public:
        HeapTimer(){
            this->heap_.reserve(64);
        }
        ~HeapTimer(){
            this->clear();
        }
        void adjust(int id, int newExpires);
        void add(int id,int time_out,const timeoutCallBack& call_bakc);
        void doWork(int id);
        void clear();
        void pop();
        void tick();
        int GetNextTick();
    private:
        void del_(size_t index);
        void shift_up_(size_t i);
        bool shift_down_(size_t index,size_t n);
        void swap_node_(size_t i,size_t j);
        std::vector<timer> heap_;
        std::unordered_map<int,size_t> ref_;
              
};
#endif