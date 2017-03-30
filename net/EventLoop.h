#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <functional>
#include "base/CurrentThread.h"

namespace ouge::net {

class EventLoop : boost::noncopyable {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void loop();
  void assertInLoopThread();
  bool isInLoopThread() const;
  static EventLoop* getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  bool looping_;
  const pid_t threadId_;
};

inline void EventLoop::assertInLoopThread() {
  if (!isInLoopThread()) {
    abortNotInLoopThread();
  }
}

inline bool EventLoop::isInLoopThread() const {
  return threadId_ == CurrentThread::tid();
}
}

#endif /* EVENTLOOP_H */
