#ifndef BASE_BOUNDEDBLOCKINGQUEUE_H
#define BASE_BOUNDEDBLOCKINGQUEUE_H

#include "base/Copyable.h"

#include <boost/circular_buffer.hpp>
#include <mutex>
#include <condition_variable>
#include <cassert>

namespace ouge {

// 使用 boost 中的环形缓冲区作为阻塞队列怕；【77777
template <typename T>
class BoundedBlockingQueue : NonCopyable {
  public:
    explicit BoundedBlockingQueue(int maxSize)
            : mutex_(), notEmpty_(), notFull_(), queue_(maxSize) {}

    void put(const T& x) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, !queue_.full());
        assert(!queue_.full());
        queue_.push_back(x);
        notEmpty_.notify_one();
    }

    void put(T&& x) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, !queue_.full());
        assert(!queue_.full());
        queue_.push_back(move(x));
        notEmpty_.notify_one();
    }

    T take() {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, !queue_.empty());
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        notFull_.notify_one();
        return front;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    bool full() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.full();
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    size_t capacity() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.capacity();
    }

  private:
    mutable std::mutex        mutex_;
    std::condition_variable   notEmpty_;
    std::condition_variable   notFull_;
    boost::circular_buffer<T> queue_;
};
}    // namespace ouge

#endif    // BASE_BOUNDEDBLOCKINGQUEUE_H