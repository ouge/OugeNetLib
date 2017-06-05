#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "base/Copyable.h"

#include <cassert>
#include <queue>

namespace ouge {

template <typename T>
class BlockingQueue : NonCopyable {
  public:
    BlockingQueue() : mutex_(), notEmpty_(mutex_), queue_() {}

    void put(const T& x) {
        MutexLockGuard lock(mutex_);
        queue_.push(std::move(x));
        notEmpty_.notify();
    }

    void put(T&& x) {
        MutexLockGuard lock(mutex_);
        queue_.push(std::move(x));
        notEmpty_.notify();
    }

    T take() {
        MutexLockGuard lock(mutex_);
        while (queue_.empty()) {
            notEmpty_.wait();
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop();
        return front;
    }

    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

  private:
    mutable MutexLock mutex_;
    Condition         notEmpty_;
    std::queue<T>     queue_;
};
}

#endif /* BLOCKINGQUEUE_H */
