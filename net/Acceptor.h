#ifndef NET_ACCEPTOR_H
#define NET_ACCEPTOR_H

#include "base/Copyable.h"
#include "net/Channel.h"
#include "net/Socket.h"

#include <functional>

namespace ouge {
namespace net {

class EventLoop;
class InetAddress;

// 封装 accept
class Acceptor : NonCopyable {
  public:
    using NewConnectionCallback =
            std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }
    bool listenning() const { return listenning_; }
    void listen();

  private:
    void handleRead();

    EventLoop*            loop_;
    Socket                acceptSocket_;
    Channel               acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool                  listenning_;
    int                   idleFd_;
};
}
}

#endif    // NET_ACCEPTOR_H
