#ifndef NET_EVENTLOOPTHREAD_H
#define NET_EVENTLOOPTHREAD_H

#include "base/Thread.h"

#include "base/Copyable.h"

#include <mutex>
#include <condition_variable>

namespace ouge {

namespace net {

class EventLoop;

class EventLoopThread : NonCopyable {
  public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb   = ThreadInitCallback(),
                    const std::string&        name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();

  private:
    void threadFunc();

    EventLoop*                  loop_;
    bool                        exiting_;
    Thread                      thread_;
    std::mutex                  mutex_;
    std::condition_variable_any cond_;
    ThreadInitCallback          callback_;
};
}    // namespace ouge::net
}    // namespace ouge

#endif    // NET_EVENTLOOPTHREAD_H
