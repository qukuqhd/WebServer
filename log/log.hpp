/**
 * @file log.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 
 * @version 0.1
 * @date 2024-05-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#ifndef _LOG_H_
#define _LOG_H_
#include "../buffer/buffer.hpp"
#include "blockQueue.hpp"
#include <memory>
#include <sys/time.h>
#include <cstdarg>
#include <thread>
enum LOG_LEVEL{
    DEBUG = 0,
    INFO = 1,
    WARN = 2 ,
    ERROR = 3
};
class Log{
    public:
        void init(int level,const char* path = "./log",const char* suffix = ".log",int maxQueueSize = 1024);
        static Log* Instance();
        static void FlushLogThread();
        void write(int level,const char * format ,...);
        void flush();
        int GetLevel();
        void setLevel(int level);
        bool IsOpen(){
            return isOpen_;
        }
    private:
        Log();
        void AppendLogLevelTitle(int levle);
        virtual ~Log();
        void AsyncWrite();
    private:
        static const int LOG_PATH_LEN = 256;
        static const int LOG_NAME_LEN = 256;
        static const int MAX_LINES = 50000;
        const char *path;
        const char *suffix;
        int MAX_LINES_;
        int lineCount_;
        int toDay_;
        bool isOpen_;
        Buffer buff_;
        int level_;
        bool isAsync_;
        FILE *fp;
        std::unique_ptr<BlockQueue<std::string>> deque_;
        std::unique_ptr<std::thread> writeThread_;
        std::mutex mtx_;
};
#define LOG_BASE(level,format,...)\
    do\
    {\
        Log *log = Log::Instance();\
        if(log->isOpen()&&log->GetLevel()<=level){\
            log->write(level,format,##__VA_ARGS__)\
            log->flush()\
        }\
    } while (0);
#define LOG_DEBUG(format,...)do{LOG_BASE(DEBUG,format,##__VA_ARGS__)} while (0);

#define LOG_INFO(format,...)do{LOG_BASE(INFO,format,##__VA_ARGS__)} while (0);

#define LOG_WARN(format,...)do{LOG_BASE(WARN,format,##__VA_ARGS__)} while (0);

#define LOG_ERROR(format,...)do{LOG_BASE(ERROR,format,##__VA_ARGS__)} while (0);

#endif
