#ifndef BASE_EVENTLOOP_H
#define BASE_EVENTLOOP_H

#include "base/Copyable.h"
#include "base/CurrentThread.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"

#include <mutex>
#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace ouge::net {

class Channel;
class Poller;
class TimerQueue;
class TimerId;

// 一个线程中最多只有一个EventLoop
class EventLoop : NonCopyable {
  public:
    EventLoop();
    ~EventLoop();

    // 只在EventLoop对象的创建线程（IO线程）调用
    void loop();

    // 从loop()中退出，线程安全。
    void quit();

    // 判断是否在IO线程当中。
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    int64_t iteration() const { return iteration_; }

    using Functor = std::function<void()>;

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loo-, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(const Functor& cb);
    void runInLoop(Functor&& cb);

    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
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

    int64_t     iteration_;
    const pid_t threadId_;
    Timestamp   pollReturnTime_;

    std::unique_ptr<Poller>     poller_;        //
    std::unique_ptr<TimerQueue> timerQueue_;    //

    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    std::unique_ptr<Channel> wakeupChannel_;
    int                      wakeupFd_;

    // scratch variables
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    Channel*    currentActiveChannel_;

    mutable std::mutex   mutex_;
    std::vector<Functor> pendingFunctors_;    // @GuardedBy mutex_

    boost::any context_;
};
}

#endif /* EVENTLOOP_H */
