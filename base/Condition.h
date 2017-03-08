#ifndef CONDITION_H
#define CONDITION_H

#include "Mutex.h"

#include <pthread.h>
#include <boost/noncopyable.hpp>

namespace ouge {

class Condition : boost::noncopyable {
 public:
  explicit Condition(MutexLock& mutex) : mutex_(mutex) {
    pthread_cond_init(&pcond_, NULL);
  }
  ~Condition() { pthread_cond_destroy(&pcond_); }
  void wait() {
    MutexLock::UnassignGuard ug(mutex_); // TODO: ????
    pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
  }
  bool waitForSeconds(double seconds);
  void notify() { pthread_cond_signal(&pcond_); }
  void notifyAll() { pthread_cond_broadcast(&pcond_); }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
};
}

#endif /* CONDITION_H */
