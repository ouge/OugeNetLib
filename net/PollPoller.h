#ifndef NET_POLLER_POLLPOLLER_H
#define NET_POLLER_POLLPOLLER_H

#include "net/Poller.h"

#include <vector>

struct pollfd;

namespace ouge {
namespace net {

class PollPoller : public Poller {
  public:
    PollPoller(EventLoop* loop);
    virtual ~PollPoller();

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
    virtual void updateChannel(Channel* channel);
    virtual void removeChannel(Channel* channel);

  private:
    void fillActiveChannels(int numEvents, ChannelList* actieChannels) const;

    using PollFdList = std::vector<struct pollfd>;
    PollFdList pollfds_;
};
}
}

#endif
