#include "heap_timer.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <utility>
void HeapTimer::adjust(int id, int newExpires){
    assert(!heap_.empty()&&ref_.count(id));
    heap_[ref_[id]].expires = Clock::now()+Ms(newExpires);
    shift_down_(ref_[id],heap_.size());
}
void HeapTimer::add(int id,int time_out,const timeoutCallBack& call_bakc){
    assert(id>0);
    size_t i;
    if(ref_.count(id)){
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back(timer{id,Clock::now()+Ms(time_out),call_bakc});
    }
    else{
        i = ref_[id];
        heap_[i].expires = Clock::now()+Ms(time_out);
        if(shift_down_(i, heap_.size())){
            shift_up_(i);
        }
    }
}
void HeapTimer::doWork(int id){
    if (heap_.empty()||!ref_.count(id)){
        return;
    }
    size_t i = ref_[id];
    auto timer = heap_[i];
    timer.call_back();
    del_(id);
}
void HeapTimer::clear(){
    heap_.clear();
    ref_.clear();
}
void HeapTimer::pop(){
    assert(!heap_.empty());
    del_(0);
}
void HeapTimer::tick(){
    if(heap_.empty()){
        return;
    }
    while(!heap_.empty()){
        auto timer = heap_.front();
        if(std::chrono::duration_cast<Ms>(timer.expires-Clock::now()).count()>0){
            break;
        }
        timer.call_back();
        pop();
    }
}
int HeapTimer::GetNextTick(){
    tick();
    size_t res = -1;
    if(!heap_.empty()){
        res = std::chrono::duration_cast<Ms>(heap_.front().expires-Clock::now()).count();
        if(res < 0){
            res = 0;
        }
    }
    return res;
}
void HeapTimer::del_(size_t index){
    assert(!heap_.empty()&&index>=0&&index<heap_.size());
    size_t i = index;
    size_t n = heap_.size() - i;
    assert(i<=n);
    if (i<n){
        swap_node_(i, n);
        if(!shift_down_(i, n)){
            shift_up_(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}
void HeapTimer::shift_up_(size_t i){
    assert(i>0&&i<heap_.size());
    size_t j  = (i-1)/2;
    while(j>=0){
        if(heap_[j]<heap_[i]){
            break;
        }
        swap_node_(i,j);
        i = j;
        j = (i-1)/2;
    }
}
bool HeapTimer::shift_down_(size_t index,size_t n){
    assert(index>0&&index<heap_.size());
    assert(n>0&&n<heap_.size());
    size_t i = index;
    size_t j = i*2+1;
    while (j<n) {
        if(j+1<n&&heap_[j+1]<heap_[j]){
            j++;
        }
        swap_node_(i, j);
        i = j;
        j = i * 2 +1;
    }
    return i>index;
}
void HeapTimer::swap_node_(size_t i,size_t j){
    assert(i>0&&i<heap_.size());
    assert(j>0&&j<heap_.size());
    std::swap(heap_[i],heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;   

}