#ifndef NET_POLLER_H
#define NET_POLLER_H

#include "base/Timestamp.h"
#include "net/EventLoop.h"
#include "base/Copyable.h"

#include <map>
#include <vector>

namespace ouge {
namespace net {

class Channel;

class Poller : NonCopyable {
  public:
    using ChannelList = std::vector<Channel*>;
    using ChannelMap  = std::map<int, Channel*>;

  public:
    Poller(EventLoop* loop);
    virtual ~Poller();

    /// Polls the I/O events.
    /// Must be called in the loop thread.
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    /// Changes the interested I/O events.
    /// Must be called in the loop thread.
    virtual void updateChannel(Channel* channel) = 0;

    /// Remove the channel, when it destructs.
    /// Must be called in the loop thread.
    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }

  protected:
    Poller::ChannelMap channels_;

  private:
    EventLoop* ownerLoop_;
};
}
}

#endif
