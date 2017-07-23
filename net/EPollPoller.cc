#include "net/EPollPoller.h"

#include "net/Channel.h"

#include <cassert>
#include <cerrno>
#include <iostream>
#include <sys/epoll.h>

using namespace ouge;
using namespace ouge::net;

namespace {
const int kNew     = -1;
const int kAdded   = 1;
const int kDeleted = 2;
}    // namesapce

EPollPoller::EPollPoller(EventLoop* loop)
        : Poller(loop),
          epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
          events_(kInitEventListSize) {
    if (epollfd_ < 0) { std::cerr << "EPollPoller::EPollPoller"; }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    std::cout << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                                 static_cast<int>(events_.size()), timeoutMs);
    int       savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        std::cout << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
        if (implicit_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        std::cout << "nothing happended";
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            std::cout << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int          numEvents,
                                     ChannelList* activeChannels) const {
    assert(implicit_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        int      fd      = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    std::cout << "fd = " << channel->fd() << " events = " << channel->events()
              << " index = " << index;
    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    std::cout << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);
    if (index == kAdded) { update(EPOLL_CTL_DEL, channel); }
    channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events   = channel->events();
    event.data.ptr = channel;
    int fd         = channel->fd();
    std::cout << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString()
              << " }";
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            std::cerr << "epoll_ctl op =" << operationToString(operation)
                      << " fd =" << fd;
        } else {
            std::cerr << "epoll_ctl op =" << operationToString(operation)
                      << " fd =" << fd;
        }
    }
}

const char* EPollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD: return "ADD";
        case EPOLL_CTL_DEL: return "DEL";
        case EPOLL_CTL_MOD: return "MOD";
        default: assert(false && "ERROR op"); return "Unknown Operation";
    }
}
