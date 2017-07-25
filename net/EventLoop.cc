#include "net/EventLoop.h"
#include "net/Channel.h"
#include "net/Poller.h"
#include "net/SocketsOps.h"
#include "net/TimerQueue.h"
#include "net/TimerId.h"

#include <cassert>
#include <iostream>
#include <functional>

#include <signal.h>
#include <sys/eventfd.h>

using namespace std;
using namespace ouge;
using namespace ouge::net;

namespace {
// 每个线程一个EventLoop
thread_local EventLoop* t_loopInThisThread = 0;
// 最长阻塞再epoll/poll的时间
const int kPollTimeMs = 10000;

int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        std::cerr << "Failed in eventfd\n" << std::endl;
        abort();
    }
    return evtfd;
}

// SIG_IGN 使用了旧式的类型转换
#pragma GCC diagnostic ignored "-Wold-style-cast"
// 忽略SigPipe信号
class IgnoreSigPipe {
  public:
    IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}    // local namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

EventLoop::EventLoop()
        : looping_(false),
          quit_(false),
          eventHandling_(false),
          callingPendingFunctors_(false),
          threadId_(this_thread::get_id()),
          poller_(Poller::newDefaultPoller(this)),
          timerQueue_(new TimerQueue(this)),
          wakeupFd_(createEventfd()),
          wakeupChannel_(new Channel(this, wakeupFd_)),
          currentActiveChannel_(NULL) {
    std::cout << "EventLoop created " << this << " in thread " << threadId_
              << endl;
    if (t_loopInThisThread) {
        // 当前线程已经存在Eventloop。
        std::cerr << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_ << endl;
    } else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    std::cout << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << this_thread::get_id() << endl;
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_    = false;
    std::cout << "EventLoop " << this << " start looping";

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        printActiveChannels();
        eventHandling_ = true;
        for (ChannelList::iterator it = activeChannels_.begin();
             it != activeChannels_.end(); ++it) {
            currentActiveChannel_ = *it;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_        = false;
        doPendingFunctors();
    }

    std::cout << "EventLoop " << this << " stop looping";
    looping_ = false;
}

// 从loop中退出
void EventLoop::quit() {
    quit_ = true;
    
    if (!isInLoopThread()) { wakeup(); }
}

void EventLoop::runInLoop(const Functor& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}
void EventLoop::runInLoop(Functor&& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(const Functor& cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_) { wakeup(); }
}

void EventLoop::queueInLoop(Functor&& cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    // 如果是在Loop thread 中，则此时一定正在执行channl回调函数或者pending队列的函数。
    // 执行channal回调时无须wakeup，因为稍后一定会立即执行cb。
    if (!isInLoopThread() || callingPendingFunctors_) { wakeup(); }
}

size_t EventLoop::queueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pendingFunctors_.size();
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

TimerId EventLoop::runAt(const Timestamp& time, TimerCallback&& cb) {
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback&& cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback&& cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) { return timerQueue_->cancel(timerId); }

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_) {
        assert(currentActiveChannel_ == channel
               || std::find(activeChannels_.begin(), activeChannels_.end(),
                            channel)
                          == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
    cerr << "EventLoop::abortNotInLoopThread - EventLoop " << this
         << " was created in threadId_ = " << threadId_
         << ", current thread id = " << this_thread::get_id() << endl;
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t  n   = sockets::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        std::cerr << "EventLoop::wakeup() writes " << n
                  << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t  n   = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        cerr << "EventLoop::handleRead() reads " << n << " bytes instead of 8"
             << endl;
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) { functors[i](); }

    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
    for (const auto& channelPtr : activeChannels_) {
        cout << "{" << channelPtr->reventsToString() << "}" << endl;
    }
}
