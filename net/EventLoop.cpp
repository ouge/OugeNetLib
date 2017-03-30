#include "net/EventLoop.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"

#include <poll.h>
#include <cassert>

namespace {

__thread ouge::net::EventLoop* t_loopInThisThread = 0;
}

namespace ouge::net {
EventLoop::EventLoop() : looping_(false), threadId_(CurrentThread::tid()) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << this << " in thread "
              << t_loopInThisThread << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
}

EventLoop::~EventLoop() {
  assert(!looping_);
  t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

// 事件循环,在IO线程执行
void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  ::poll(NULL, 0, 5 * 1000);
  LOG_TRACE << "EventLoop "  << this << " stop looping";
  looping_ = false;
}
}