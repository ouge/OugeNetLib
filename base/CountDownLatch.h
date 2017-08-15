#ifndef BASE_COUNTDOWNLATCH_H
#define BASE_COUNTDOWNLATCH_H

#include "base/Copyable.h"

#include <mutex>
#include <condition_variable>

namespace ouge {

// 线程安全的计数器
class CountDownLatch : NonCopyable {
  public:
    explicit CountDownLatch(int count)
            : mutex_(), condition_(), count_(count) {}

    // wait直到倒计时为0
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] {
            return count_ <= 0;    //  <= 0 而不是 <
        });
    }

    // 倒计时
    void countDown() {
        std::unique_lock<std::mutex> lock(mutex_);
        --count_;
        if (count_ == 0) { condition_.notify_all(); }
    }

    // 返回当前计时数
    int getCount() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return count_;
    }

  private:
    // 互斥锁,保证对 count_ 的访问的互斥性
    mutable std::mutex mutex_;
    // 条件变量
    std::condition_variable condition_;
    int                     count_;
};
}    // namespace ouge

#endif    // BASE_COUNTDOWNLATCH_H