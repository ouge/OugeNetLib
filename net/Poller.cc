#include "net/Poller.h"

#include "net/Channel.h"

using namespace ouge;
using namespace ouge::net;

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {}

Poller::~Poller() {}

bool
Poller::hasChannel(Channel* channel) const {
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
