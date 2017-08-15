#include "net/Acceptor.h"

#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOps.h"

#include <functional>
#include <cerrno>
#include <iostream>
#include <cassert>
#include <fcntl.h>

using namespace ouge;
using namespace ouge::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr,
                   bool reuseport)
        : loop_(loop),
          acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
          acceptChannel_(loop, acceptSocket_.fd()),
          listenning_(false),
          idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    // 使所属线程观察listenfd的读事件
    acceptChannel_.enableReading();
}

// 实际上就是新连接建立后的回调
void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr;

    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    } else {
        std::cerr << "in Acceptor::handleRead" << std::endl;

        // 如果 fd 超最大描述符上限，解决方案是关闭连接
        // 然而此时拿不到连接的描述符，因此可以使用 idleFd 的那个fd。
        if (errno == EMFILE) {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            // 使用fd完后需归还，以便之后重复使用
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}