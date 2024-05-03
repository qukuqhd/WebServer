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
#include <iostream>
Buffer::Buffer(size_t initBufferSize):buffer_(initBufferSize),read_pos_(0),write_pos_(0){

}
size_t Buffer::ReadableBytes() const{
    return write_pos_ - read_pos_;//返回可以写的位置和读的位置之间的长度
}

size_t Buffer::WriteableBytes() const{
    return buffer_.size() - write_pos_;//返回现在缓存的大小减去写入的位置
}

size_t Buffer::PrependableBytes() const{
    return read_pos_;//返回读取的位置
}
const char* Buffer::Peek() const{
    return BeginPtr_() + read_pos_;//返回缓存开始地址加上读取的位置
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
    bzero(&buffer_[0], buffer_.size());//清空缓存的内容
    //初始化读写位置
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::RetrieveAllToStr(){
    std::string str(Peek(),ReadableBytes());//创建缓冲剩余长度的字符串
    RetrieveAll();//清空缓冲
    return str;
}

const char* Buffer::BeginWriteConst() const{
    return BeginPtr_() + write_pos_;//获取写位置的指针
}

char* Buffer::BeginWrite(){
    return BeginPtr_() + write_pos_;//获取写位置的指针
}

void Buffer::HasWritten(size_t len){
    write_pos_ += len;//已经写入修改写的位置
}

void Buffer::Append(const std::string& str){
    Append(str.data(),str.size());//添加字符串到缓存
}

void Buffer::Append(const char* str,size_t len){
    assert(str);//断言判断
    EnsureWriteable(len);//确保缓存空间足够写入
    std::copy(str,str + len, BeginWrite());//拷贝字符串到缓存写入的开始地址
    HasWritten(len);//已经写入修改写入位置
}

void Buffer::Append(const void* data,size_t len){
    assert(data);//断言判断
    Append(static_cast<const char*>(data),len);//转换为字符指针进行写入
}

void Buffer::Append(const Buffer& buff){
    Append(buff.Peek(),buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len){
    if(WriteableBytes() < len){//可以写入的长度小就进行扩容
        MakeSpace_(len);
    }
    assert(WriteableBytes() >= len);//断言判断扩容之后的可写长度足够
}

ssize_t Buffer::ReadFd(int fd,int* savedErrno){
    char buff[65536];
    struct iovec iov[2];
    const size_t writable = WriteableBytes();
    /**进行IO读写的分散**/
    iov[0].iov_base = BeginPtr_() + write_pos_;//写入位置的指针作为缓存开始
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;//以缓存类的开始作为开始
    iov[1].iov_len = sizeof(buff);
    const ssize_t len = readv(fd,iov,2);//分聚读取文件描述符的内容到
    if(len < 0){//判断是否读取错误
        *savedErrno = errno;
    }else if(static_cast<size_t>(len) <= writable){//读取的内容全部到了缓存类里面了
        write_pos_ += len;
    }else{//有数据读取到了临时的缓存变量里面了
        write_pos_ = buffer_.size();
        Append(buff,len - writable);
    }
    return len;
}   

ssize_t Buffer::WriteFd(int fd,int* savedErrno){
    size_t read_size = ReadableBytes();//获取可以读取缓存的长度
    ssize_t len = write(fd,Peek(),read_size);//把缓存内容写入到文件
    if(len <= 0){//写入失败
        *savedErrno = errno;
        return len;
    }
    read_pos_ += len;//读取位置修改
    return len;
}

ssize_t Buffer::ReadFile(FILE *fp) {     
    char buff[65536];
    int index = 0;
    while (true)
    {
        int character = fgetc(fp);
        if(character==EOF){
            break;
        }
        buff[index++] = character;
    }
    Append(buff,index); 
    return index;
}

ssize_t Buffer::WriteFile(FILE *fp) { 
    size_t write_size =  fwrite(BeginPtr_()+write_pos_,1,ReadableBytes(),fp);
    return write_size;
 }

 char *Buffer::BeginPtr_() { return &*buffer_.begin(); }

 const char *Buffer::BeginPtr_() const { return &*buffer_.begin(); }

 void Buffer::MakeSpace_(size_t len) {
   if (WriteableBytes() + PrependableBytes() <
       len) { // 缓存整个的空间不足就考虑孔融
     buffer_.resize(write_pos_ + len);
   } else { // 缓存的空间足够就进行数据的移动
     size_t readable = ReadableBytes();
     std::copy(BeginPtr_() + read_pos_, BeginPtr_() + write_pos_,
               BeginPtr_());            // 拷贝数据到缓存的开始
     read_pos_ = 0;                     // 读取位置归零
     write_pos_ = read_pos_ + readable; // 写入位置为当前缓存数据的长度
   }
}