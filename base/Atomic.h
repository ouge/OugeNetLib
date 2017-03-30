#ifndef ATOMIC_H
#define ATOMIC_H

#include <boost/noncopyable.hpp>
#include <cstdint>

namespace ouge::detail {

template <typename T>
class AtomicIntegerT : boost::noncopyable {
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
