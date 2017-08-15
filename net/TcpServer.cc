#include "net/TcpServer.h"

#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/EventLoopThreadPool.h"
#include "net/SocketsOps.h"

#include <cstdio>
#include <functional>
#include <iostream>

using namespace std;
using namespace ouge;
using namespace ouge::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                     const std::string& nameArg, Option option)
        : loop_(loop),
          ipPort_(listenAddr.toIpPort()),
          name_(nameArg),
          acceptor_(new Acceptor(loop, listenAddr, olllption == kReusePort)),
          threadPool_(new EventLoopThreadPool(loop, name_)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          nextConnId_(1) {
    assert(loop != nullptr);
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                  this, std::placeholders::_1,
                                                  std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    std::cout << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (ConnectionMap::iterator it(connections_.begin());
         it != connections_.end(); ++it) {
        TcpConnectionPtr conn = it->second;
        it->second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
        conn.reset();
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_.exchange(1) == 0) {
        threadPool_->start(threadInitCallback_);

        assert(!acceptor_->listenning());
        loop_->runInLoop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    // 从线程池中取出一个 EventLoop；
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char       buf[64];
    // 每个连接的名字：服务器名+服务器端口+递增id
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    cout << "TcpServer::newConnection [" << name_ << "] - new connection ["
         << connName << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // 创建一个新TcpConnection 并和之前取得的 EventLop关联
    // 也就让该EventLoop所属线程来处理这个连接的IO。
    TcpConnectionPtr conn(
            new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    // 将新连接TcpConnectionPtr 插入 map
    connections_[connName] = conn;
    // 设置回调函数
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, placeholders::_1));
    // 让连接所属线程在其线程栈上执行连接建立回调
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    cout << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection "
         << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
