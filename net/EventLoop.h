#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "base/Copyable.h"
#include "base/CurrentThread.h"
#include "base/Mutex.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"

#include <boost/any.hpp>
#include <boost/scoped_ptr.hpp>
#include <functional>
#include <vector>

namespace ouge::net {

class Channel;
class Poller;
class TimerQueue;
class TimerId;

// 一个线程中最多只有一个EventLoop
// 不可复制
class EventLoop : NonCopyable {
 public:
  using Functor = std::function<void()>;

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

  TimerId runAt(const Timestamp& time, TimerCallback&& cb);

  TimerId runAfter(double delay, TimerCallback&& cb);
  TimerId runEvery(double interval, TimerCallback&& cb);

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

  using ChannelList = std::vector<Channel*>;

  bool looping_;
  bool quit_;
  bool eventHandling_;
  bool callingPendingFunctors_;

  boost::scoped_ptr<Poller> poller_;
  boost::scoped_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  boost::scoped_ptr<Channel> wakeupChannel_;
  boost::any context_;

  // scratch variables
  ChannelList activeChannels_;
  Channel* currentActiveChannel_;

  mutable MutexLock mutex_;
  std::vector<Functor> pendingFunctors_;  // @GuardedBy mutex_

  const pid_t threadId_;
};
}

#endif /* EVENTLOOP_H */
