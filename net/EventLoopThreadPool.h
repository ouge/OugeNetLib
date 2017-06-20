#ifndef NET_EVENTLOOPTHREADPOOL_H
#define NET_EVENTLOOPTHREADPOOL_H

#include "base/Types.h"
#include "base/Copyable.h"

#include <vector>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>

namespace ouge {

namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : NonCopyable {
  public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // valid after calling start()
    /// round-robin
    EventLoop* getNextLoop();

    /// with the same hash code, it will always return the same EventLoop
    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }

    const std::string& name() const { return name_; }

  private:
    EventLoop*                         baseLoop_;
    std::string                        name_;
    bool                               started_;
    int                                numThreads_;
    int                                next_;
    boost::ptr_vector<EventLoopThread> threads_;
    std::vector<EventLoop*>            loops_;
};
}
}

#endif    // MUDUO_NET_EVENTLOOPTHREADPOOL_H
