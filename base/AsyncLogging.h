#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#include "Condition.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

namespace ouge {
class AsyncLogging : boost::noncopyable {
 public:
  AsyncLogging(const std::string& basename, size_t rollSize,
               int flushInterval = 3);
  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }
  void append(const char* logline, int len);
  void start();
  void stop();


 private:
  AsyncLogging(const AsyncLogging&);
  void operator=(const AsyncLogging&);
  void threadFunc();

  typedef ouge::detail::FixedBuffer<ouge::detail::kLargeBuffer> Buffer;

  const int flushInterval_;
  bool running_;
  std::string basename_;
  size_t rollSize_;
  ouge::Thread thread_;
  ouge::CountDownLatch latch_;
  ouge::MutexLock mutex_;
  ouge::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};
}

#endif /* ASYNCLOGGING_H */
