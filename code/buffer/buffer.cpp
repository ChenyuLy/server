/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "buffer.h"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

size_t Buffer::ReadableBytes() const {
    return writePos_ - readPos_;
}
size_t Buffer::WritableBytes() const {
    return buffer_.size() - writePos_;
}

size_t Buffer::PrependableBytes() const {
    return readPos_;
}

const char* Buffer::Peek() const {
    return BeginPtr_() + readPos_;
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end );
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writePos_;
}

char* Buffer::BeginWrite() {
    return BeginPtr_() + writePos_;
}

void Buffer::HasWritten(size_t len) {
    writePos_ += len;
} 

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);       //查看能否写入 增加缓存长度
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    if(WritableBytes() < len) {
        MakeSpace_(len); //长度不够创建长度 函数的具体实现还不理解
    }
    assert(WritableBytes() >= len);
}

// iovec结构体定义如下
// struct iovec {
//     void *iov_base; // 缓冲区起始地址
//     size_t iov_len; // 缓冲区长度
// };


ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
    char buff[65535];                       //新创建一个临时buffer
    struct iovec iov[2];
    const size_t writable = WritableBytes(); //查看类中定义的缓存区还有多少的空间剩余 用buffer的大小减去写指针的位置
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;  //0 代表类定义的buffer剩余部分
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;                     //临时buffer
    iov[1].iov_len = sizeof(buff);
// iov:iovec结构体数组的地址值(多个缓冲区数据整合一并发送)
// iovcnt:第二个参数iov数组的长度
    const ssize_t len = readv(fd, iov, 2);          //使用合并的两个缓存区接受数据
    if(len < 0) {                                   //接收失败
        *saveErrno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) { // 接收长度在第一个buff的范围内
        writePos_ += len; 
    }
    else {                                          //接收长度到了第二个buff
        writePos_ = buffer_.size();
        Append(buff, len - writable);               //传入第二个buff中缓存的长度
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *saveErrno = errno;
        return len;
    } 
    readPos_ += len;
    return len;
}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writePos_ + len + 1);
    } 
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}