#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H

#include "base/Types.h"
#include "net/TcpConnection.h"

#include "base/Copyable.h"

#include <boost/scoped_ptr.hpp>
#include <map>
#include <atomic>
#include <memory>
#include <functional>

namespace ouge {
namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class TcpServer : NonCopyable {
  public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option {
        kNoReusePort,
        kReusePort,
    };

    // TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    TcpServer(EventLoop* loop, const InetAddress& listenAddr,
              const std::string& nameArg, Option option = kNoReusePort);
    ~TcpServer();    // force out-line dtor, for scoped_ptr members.

    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop*         getLoop() const { return loop_; }

    /// Set the number of threads for handling input.
    ///
    /// Always accepts new connection in loop's thread.
    /// Must be called before @c start
    /// @param numThreads
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.
    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) {
        threadInitCallback_ = cb;
    }
    /// valid after calling start()
    std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

    /// Starts the server if it's not listenning.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

  private:
    // 由Acceptor回调，创建TcpConnection对象并加入ConnectionMap中
    // 设置TcpConnection的回调函数。
    void newConnection(int sockfd, const InetAddress& peerAddr);
    /// Thread safe.
    void removeConnection(const TcpConnectionPtr& conn);
    /// Not thread safe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop*                  loop_;    // the acceptor loop
    const std::string           ipPort_;
    const std::string           name_;
    boost::scoped_ptr<Acceptor> acceptor_;    // avoid revealing Acceptor
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback                   connectionCallback_;
    MessageCallback                      messageCallback_;
    WriteCompleteCallback                writeCompleteCallback_;
    ThreadInitCallback                   threadInitCallback_;
    std::atomic_uint32_t                 started_;
    // always in loop thread
    int nextConnId_;

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;
};
}
}

#endif    // OUGE_NET_TCPSERVER_H
