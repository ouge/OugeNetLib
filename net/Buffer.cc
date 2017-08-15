#include "net/Buffer.h"

#include "net/SocketsOps.h"

#include <cerrno>
#include <sys/uio.h>

using namespace ouge;
using namespace ouge::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

// 读 fd
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char         extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    // 第一块内存是来自Buffer自带的vector<char>
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len  = writable;
    // 第二块内存是来自本函数栈上
    vec[1].iov_base = extrabuf;
    vec[1].iov_len  = sizeof extrabuf;

    // 如果Buffer剩余空间足够,不会启动栈上空间
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    // 散列读
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (implicit_cast<size_t>(n) <= writable) {
        // 数据全读进了 vector
        writerIndex_ += n;
    } else {
        // 部分数据保存在栈上
        writerIndex_ = buffer_.size();
        // 一次性添加
        append(extrabuf, n - writable);
    }

    return n;
}
