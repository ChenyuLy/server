/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 

#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);        //初始化buff 初始读位置和写位置都为0
    ~Buffer() = default;

    size_t WritableBytes() const;               //返回可写的buff空间大小，具体为buff大小减去 可以写位置
    size_t ReadableBytes() const ;              //返回可读的buff空间大小，即 写位置减去 读位置的大小   即写入后可读的位置大小
    size_t PrependableBytes() const;            //已经准备好的buff大小？ 即写位置之前

    const char* Peek() const;                   //查看起始位置到读位置的大小
    void EnsureWriteable(size_t len);           //确保buff能够写入数据 如果不能写入则创建空间
    void HasWritten(size_t len);                //写入数据以后更新 已写位置的值

    void Retrieve(size_t len);                  //用len 根新已读位置的值
    void RetrieveUntil(const char* end);        

    void RetrieveAll() ;                        //重置buff
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;      //返回起始位置的迭代器
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;      //使用Vector为作为buff
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};

#endif //BUFFER_H