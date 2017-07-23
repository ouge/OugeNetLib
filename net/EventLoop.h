#ifndef BASE_EVENTLOOP_H
#define BASE_EVENTLOOP_H

#include "base/Copyable.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"

#include <mutex>
#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <thread>

namespace ouge

{
namespace net {
class Channel;
class Poller;
class TimerQueue;
class TimerId;

// 一个线程中最多只有一个EventLoop
class EventLoop : NonCopyable {
  public:
    using Functor     = std::function<void()>;
    using ChannelList = std::vector<Channel*>;

  public:
    EventLoop();
    ~EventLoop();

    // 只在EventLoop对象的创建线程（IO线程）调用
    void loop();

    // 从loop()中退出，线程安全。
    void quit();

    // 判断是否在IO线程当中。
    void assertInLoopThread() {
        if (!isInLoopThread()) { abortNotInLoopThread(); }
    }
    bool isInLoopThread() const {
        return threadId_ == std::this_thread::get_id();
    }

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(const Functor& cb);
    void runInLoop(Functor&& cb);

    void queueInLoop(const Functor& cb);
    void queueInLoop(Functor&& cb);

    size_t queueSize() const;

    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);

    TimerId runAt(const Timestamp& time, TimerCallback&& cb);
    TimerId runAfter(double delay, TimerCallback&& cb);
    TimerId runEvery(double interval, TimerCallback&& cb);

    void cancel(TimerId timerId);

    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    static EventLoop* getEventLoopOfCurrentThread();

  private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();
    void printActiveChannels() const;

    bool looping_;
    bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;

    const std::thread::id threadId_;

    Timestamp               pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    std::unique_ptr<TimerQueue> timerQueue_;

    int                      wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel*    currentActiveChannel_;

    // 理论上应该立即执行的函数队列
    std::vector<Functor> pendingFunctors_;

    mutable std::mutex mutex_;

    boost::any context_;
};
}
}
#endif /* EVENTLOOP_H */
