#ifndef NET_ACCEPTOR_H
#define NET_ACCEPTOR_H

#include <boost/function.hpp>
#include "base/Copyable.h"

#include "net/Channel.h"
#include "net/Socket.h"

namespace ouge {
namespace net {

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : NonCopyable {
 public:
  typedef boost::function<void(int sockfd, const InetAddress&)>
      NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }

  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listenning_;
  int idleFd_;
};
}
}

#endif  // MUDUO_NET_ACCEPTOR_H
