#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include  "base/Copyable.h"
#include <cstdint>

namespace ouge {
namespace net {

class Timer;

class TimerId : public Copyable {
 public:
  TimerId() : timer_(nullptr), sequence_(0) {}

  TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}

  // default copy-ctor, dtor and assignment are okay

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};
}
}

#endif  // MUDUO_NET_TIMERID_H
