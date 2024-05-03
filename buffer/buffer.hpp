/**
 * @file buffer.hpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 
 * @version 0.1
 * @date 2024-05-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <sys/types.h>
#include <vector>
#include <string>
#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_
/**
 * @brief 字符缓冲类
 * 
 */
class Buffer{
    public:
        /**
         * @brief 构造函数根据初始化容量创建缓冲
         * 
         * @param initBufferSize 缓冲尺寸
         */
        Buffer(size_t initBufferSize = 1024);
        ~Buffer()=default;
        /**
         * @brief 获取可写字节数
         * 
         * @return size_t 
         */
        size_t WriteableBytes() const;
        /**
         * @brief 获取可读字节数
         * 
         * @return size_t 
         */
        size_t ReadableBytes() const;
        /**
         * @brief 获取读的位置
         * 
         * @return size_t 
         */
        size_t PrependableBytes() const;
        /**
         * @brief 获取当前读取的缓冲首地址
         * 
         * @return const char* 
         */
        const char* Peek() const;
        /**
         * @brief 取保可写的长度足够
         * 
         * @param len 
         */
        void EnsureWriteable(size_t len);
        /**
         * @brief 已写入指定长度，修改写指针
         * 
         * @param len 
         */
        void HasWritten(size_t len);
        /**
         * @brief 读取指定长度数据
         * 
         * @param len 
         */
        void Retrieve(size_t len);
        /**
         * @brief 读取到指定位置
         * 
         * @param end 
         */
        void RetrieveUntil(const char* end);
        /**
         * @brief 读取到末尾
         * 
         */
        void RetrieveAll();
        /**
         * @brief 获取缓冲剩余
         * 
         * @return std::string 
         */
        std::string RetrieveAllToStr();
        /**
         * @brief 获取缓冲写首地址
         * 
         * @return const char* 
         */
        const char* BeginWriteConst() const;
        /**
         * @brief 获取缓冲写首地址
         * 
         * @return char* 
         */
        char* BeginWrite();
        /**
         * @brief 缓冲写入字符串
         * 
         * @param str 
         */
        void Append(const std::string& str);
        /**
         * @brief 缓冲写入字符串
         * 
         * @param str 
         * @param len 
         */
        void Append(const char* str, size_t len);
        /**
         * @brief 缓冲写入
         * 
         * @param data 
         * @param len 
         */
        void Append(const void* data, size_t len);
        /**
         * @brief 缓冲写入
         * 
         * @param buffer 
         */
        void Append(const Buffer& buffer);
        /**
         * @brief 读取文件描述符
         * 
         * @param fd 
         * @param savedErrno 
         * @return ssize_t 
         */
        ssize_t ReadFd(int fd, int* savedErrno);
        /**
         * @brief 写入文件描述符
         * 
         * @param fd 
         * @param savedErrno 
         * @return ssize_t 
         */
        ssize_t WriteFd(int fd, int* savedErrno);
        /**
         * @brief 读取文件指针内容到缓存
         * 
         * @param fp 
         * @return ssize_t 
         */
        ssize_t ReadFile(FILE* fp);
        /**
         * @brief 写入缓存内容到文件指针的文件
         * 
         * @param fp 
         * @return ssize_t 
         */
        ssize_t WriteFile(FILE*fp);
    private:
        char* BeginPtr_();
        const char* BeginPtr_() const;
        /**
         * @brief 创建空间
         * 
         * @param len 
         */
        void MakeSpace_(size_t len);
        std::vector<char> buffer_;//缓冲存储的容器
        std::atomic<std::size_t> read_pos_;//读取位置原子变量
        std::atomic<std::size_t> write_pos_;//写入位置原子变量
};
#endif