#ifndef ATOMIC_H
#define ATOMIC_H

#include <cstdint>
#include "base/Copyable.h"

namespace ouge::detail {

template <typename T>
class AtomicIntegerT : NonCopyable {
 public:
  AtomicIntegerT();
  T get();
  T getAndAdd(T x);
  T addAndGet(T x);
  T incrementAndGet();
  T decrementAndGet();
  void add(T x);
  void increment();
  void decrement();
  T getAndSet(T newValue);

 private:
  volatile T value_;
};
}

namespace ouge {

using AtomicInt32 = detail::AtomicIntegerT<int32_t>;
using AtomicInt64 = detail::AtomicIntegerT<int64_t>;
}

#endif /* ATOMIC_H */
