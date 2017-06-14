#ifndef BASE_COUNTDOWNLATCH_H
#define BASE_COUNTDOWNLATCH_H

#include "base/Copyable.h"

#include <mutex>
#include <condition_variable>

namespace ouge {

// 
class CountDownLatch : NonCopyable {
  public:
    explicit CountDownLatch(int count)
            : mutex_(), condition_(), count_(count) {}

    void wait() {
        std::lock_guard<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]() -> bool { return count_ <= 0; });
    }

    void countDown() {
        std::lock_guard<std::mutex> lock(mutex_);
        --count_;
        if (count_ == 0) {
            condition_.notify_all();
        }
    }

    int getCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

  private:
    mutable std::mutex          mutex_;
    std::condition_variable_any condition_;
    int                         count_;
};
}    // namespace ouge

#endif    // BASE_COUNTDOWNLATCH_H