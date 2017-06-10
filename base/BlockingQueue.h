#ifndef BASE_BLOCKINGQUEUE_H
#define BASE_BLOCKINGQUEUE_H

#include "base/Copyable.h"

#include <cassert>
#include <queue>
#include <condition_variable>
#include <mutex>

namespace ouge {

// 以 stl::queue 作为为阻塞队列
template <typename T>
class BlockingQueue : NonCopyable {
  public:
    BlockingQueue() : mutex_(), notEmpty_(), queue_() {}

    void put(const T& x) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(x);
        notEmpty_.notify_one();
    }

    void put(T&& x) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(std::move(x));
        notEmpty_.notify_one();
    }

    T take() {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, !queue_.empty());
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop();
        return front;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

  private:
    mutable std::mutex      mutex_;
    std::condition_variable notEmpty_;
    std::queue<T>           queue_;
};
}

#endif    // BASE_BLOCKINGQUEUE_H
