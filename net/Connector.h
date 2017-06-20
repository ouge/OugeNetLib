#ifndef NET_CONNECTOR_H
#define NET_CONNECTOR_H

#include "net/InetAddress.h"

#include "base/Copyable.h"

#include <memory>
#include <functional>
#include <boost/scoped_ptr.hpp>

namespace ouge {
namespace net {

class Channel;
class EventLoop;

class Connector : private NonCopyable,
                  public std::enable_shared_from_this<Connector> {
  public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    void start();      // can be called in any thread
    void restart();    // must be called in loop thread
    void stop();       // can be called in any thread

    const InetAddress& serverAddress() const { return serverAddr_; }

  private:
    enum States { kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs  = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_ = s; }

    void startInLoop();
    void stopInLoop();

    void connect();
    void connecting(int sockfd);

    void handleWrite();
    void handleError();

    void retry(int sockfd);
    int  removeAndResetChannel();
    void resetChannel();

    EventLoop*                 loop_;
    InetAddress                serverAddr_;
    bool                       connect_;    // atomic
    States                     state_;      // FIXME: use atomic variable
    boost::scoped_ptr<Channel> channel_;
    NewConnectionCallback      newConnectionCallback_;
    int                        retryDelayMs_;
};
}    // namespace ouge::net
}    // namespace ouge

#endif
