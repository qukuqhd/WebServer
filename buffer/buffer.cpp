/**
 * @file buffer.cpp
 * @author {gangx} ({gangx6906@gmail.com})
 * @brief 
 * @version 0.1
 * @date 2024-05-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "buffer.hpp"
#include <cassert>
#include <cstddef>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include "sys/uio.h"
Buffer::Buffer(size_t initBufferSize):buffer_(initBufferSize),read_pos_(0),write_pos_(0){

}
size_t Buffer::ReadableBytes() const{
    return write_pos_ - read_pos_;
}

size_t Buffer::WriteableBytes() const{
    return buffer_.size() - write_pos_;
}

size_t Buffer::PrependableBytes() const{
    return read_pos_;
}
const char* Buffer::Peek() const{
    return BeginPtr_() + read_pos_;
}

void Buffer::Retrieve(size_t len){
    assert(len <= ReadableBytes());//检查是否越界
    read_pos_ += len;//更新读取位置
}

void Buffer::RetrieveUntil(const char* end){
    assert(Peek() <= end);//检查参数是否在当前位置之前
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll(){
    bzero(&buffer_[0], buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::RetrieveAllToStr(){
    std::string str(Peek(),ReadableBytes());//创建缓冲剩余长度的字符串
    RetrieveAll();//清空缓冲
    return str;
}

const char* Buffer::BeginWriteConst() const{
    return BeginPtr_() + write_pos_;
}

char* Buffer::BeginWrite(){
    return BeginPtr_() + write_pos_;
}

void Buffer::HasWritten(size_t len){
    write_pos_ += len;
}

void Buffer::Append(const std::string& str){
    Append(str.data(),str.size());
}

void Buffer::Append(const char* str,size_t len){
    assert(str);
    EnsureWriteable(len);
    std::copy(str,str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const void* data,size_t len){
    assert(data);
    Append(static_cast<const char*>(data),len);
}

void Buffer::Append(const Buffer& buff){
    Append(buff.Peek(),buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len){
    if(WriteableBytes() < len){
        MakeSpace_(len);
    }
    assert(WriteableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd,int* savedErrno){
    char buff[65536];
    struct iovec iov[2];
    const size_t writable = WriteableBytes();
    iov[0].iov_base = BeginPtr_() + write_pos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    const ssize_t len = readv(fd,iov,2);
    if(len < 0){
        *savedErrno = errno;
    }else if(static_cast<size_t>(len) <= writable){
        write_pos_ += len;
    }else{
        write_pos_ = buffer_.size();
        Append(buff,len - writable);
    }
    return len;
}   

ssize_t Buffer::WriteFd(int fd,int* savedErrno){
    size_t writeable = WriteableBytes();
    ssize_t len = write(fd,Peek(),writeable);
    if(len <= 0){
        *savedErrno = errno;
        return len;
    }
    write_pos_ += len;
    return len;
}

char* Buffer::BeginPtr_(){
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const{
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len){
    if(WriteableBytes() + PrependableBytes() < len){
        buffer_.resize(write_pos_ + len);
    }else{
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + read_pos_,BeginPtr_() + write_pos_,BeginPtr_());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
    }
}