#include "net/Poller.h"
#include "net/PollPoller.h"
#include "net/EPollPoller.h"

#include <stdlib.h>

using namespace ouge::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("OUGE_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}