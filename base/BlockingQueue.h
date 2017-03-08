#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/noncopyable.hpp>
#include <cassert>
#include <deque>

namespace ouge {

template <typename T>
class BlockingQueue : boost::noncopyable {
 public:
  BlockingQueue() : mutex_(), notEmpty_(mutex_), queue_() {}
  void put(const T&);
  void put(T&&);
  T take();
  size_t size() const {}

 private:
  mutable MutexLock mutex_;
  Condition notEmpty_;
  std::deque<T> queue_;
};
}

#endif /* BLOCKINGQUEUE_H */
