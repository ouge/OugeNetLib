#include "net/Poller.h"
#include "net/PollPoller.h"
#include "net/EPollPoller.h"

#include "net/Channel.h"

#include <cstdlib>

using namespace ouge;
using namespace ouge::net;

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {}

Poller::~Poller() {}

bool Poller::hasChannel(Channel* channel) const {
    assertInLoopThread();
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("OUGE_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}
