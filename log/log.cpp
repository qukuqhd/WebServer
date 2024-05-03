#include "log.hpp"
#include "blockQueue.hpp"
#include <bits/types/struct_timeval.h>
#include <cstdarg>
#include <memory>
#include <thread>
#include <sys/stat.h>

void Log::init(int level, const char *path, const char *suffix,
               int maxQueueSize) {
    isOpen_ = true;
    level_  = level;
    if(maxQueueSize>0){
        isAsync_ = true;
        if(!deque_){
            std::unique_ptr<BlockQueue<std::string>> newDeque(new BlockQueue<std::string>(1024));
            deque_ = std::move(newDeque);
            std::unique_ptr<std::thread> NewThread(new std::thread());
            writeThread_ = std::move(NewThread);
        }
    }else{
        isAsync_ = false;
    }
    lineCount_ = 0;
    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path = path;
    suffix = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName,LOG_NAME_LEN-1,"%s/%04d_%02d_%02d%s",path,t.tm_year+1900,t.tm_mon+1,t.tm_mday,suffix);
    toDay_ = t.tm_mday;
    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.RetrieveAll();
        if(fp){
            flush();
            fclose(fp);
        }
        fp = fopen(fileName,"a");
        if (fp==nullptr){
            mkdir(path,0777);
            fp = fopen(fileName,"a");   
        }
        assert(fp!=nullptr);
    }
}
void Log::write(int level,const char *format,...){
    struct timeval now = {0,0};
    gettimeofday(&now,nullptr);
    time_t tSec = now.tv_sec;
    struct tm*sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;
    if(toDay_!=t.tm_mday||(lineCount_%MAX_LINES_)==0){
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        if(toDay_ != t.tm_mday){
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path, tail, suffix);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else{
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path, tail, (lineCount_  / MAX_LINES), suffix);
        }
        locker.lock();
        flush();
        fclose(fp);
        fp = fopen(newFile,"a");
        assert(fp!=nullptr);
    }   

    {
        std::unique_lock<std::mutex> locker(mtx_);
        lineCount_++;
        int n  = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        AppendLogLevelTitle(level);
        va_start(vaList,format);
        int m = vsnprintf(buff_.BeginWrite(),buff_.WriteableBytes(),format,vaList);
        va_end(vaList);
        buff_.HasWritten(m);
        buff_.Append("\n\0",2);
        if(isAsync_&&!deque_->full()){
            deque_->push_back(buff_.RetrieveAllToStr());
        }
        else{
            fputs(buff_.Peek(),fp);
        }
        buff_.RetrieveAll();
    }
}
void Log::AppendLogLevelTitle (int level){
    switch (level)
    {
        case LOG_LEVEL::DEBUG:{
            buff_.Append("[debug]:",9);
            break;
        }

        case LOG_LEVEL::INFO:{
            buff_.Append("[info]:",8);
            break;
        }
        case LOG_LEVEL::WARN:{
            buff_.Append("[warn]:",8);
            break;
        }
        case LOG_LEVEL::ERROR:{
            buff_.Append("[error]:",9);
            break;
        }
    }
}
void Log::flush(){
    if(isAsync_){
        deque_->full();
    }
    fflush(fp);
}
void Log::AsyncWrite(){
    std::string str = "";
    while(deque_->pop(str)){
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(),fp);
    }
}
Log* Log::Instance(){
    static Log inst;
    return &inst;
}
void Log::FlushLogThread(){
    Log::Instance()->AsyncWrite();
}
Log::~Log() {

  if (writeThread_ && writeThread_->joinable()) { // 关闭之前等待写线程结束
    while (!deque_->empty()) // 不断处理队列剩下的内容
    {
      deque_->flush();
    }
    deque_->Close(); // 处理完毕关闭队列
    writeThread_->join();
  }
  if (fp) { // 处理文件指针
    std::lock_guard<std::mutex> locker(mtx_);
    flush();
    fclose(fp);
  }
}

int Log::GetLevel(){
    std::lock_guard<std::mutex> lokcer(mtx_);
    return level_;
}

void Log::setLevel(int level){
    std::lock_guard<std::mutex> lokcer(mtx_);
    level_ = level;
}
