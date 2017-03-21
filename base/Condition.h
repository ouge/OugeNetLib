#ifndef CONDITION_H
#define CONDITION_H

#include "Mutex.h"

#include <pthread.h>
#include <boost/noncopyable.hpp>
#include <cerrno>

namespace ouge {

class Condition : boost::noncopyable {
 public:
  explicit Condition(MutexLock& mutex) : mutex_(mutex) {
    pthread_cond_init(&pcond_, NULL);
  }
  ~Condition() { pthread_cond_destroy(&pcond_); }

  void wait() {
    // pthread_cond_wait() 会对 mutex_ 先解锁。
    MutexLock::UnassignGuard ug(mutex_);
    pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
  }

  bool waitForSeconds(double seconds) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    const int64_t kNanoSecondsPerSecond = 1e9;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) /
                                          kNanoSecondsPerSecond);
    abstime.tv_nsec += static_cast<long>((abstime.tv_nsec + nanoseconds) %
                                         kNanoSecondsPerSecond);
    MutexLock::UnassignGuard ug(mutex_);
    return ETIMEDOUT ==
           pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
  }

  void notify() { pthread_cond_signal(&pcond_); }
  void notifyAll() { pthread_cond_broadcast(&pcond_); }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
};
}

#endif /* CONDITION_H */
