#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

#include "base/Copyable.h"

namespace ouge {

class CountDownLatch : NonCopyable {
 public:
  explicit CountDownLatch(int count)
      : mutex_(), condition_(mutex_), count_(count) {}

  void wait() {
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
      condition_.wait();
    }
  }

  void countDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0) {
      condition_.notifyAll();
    }
  }

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};
}

#endif /* COUNTDOWNLATCH_H */
