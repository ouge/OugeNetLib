#ifndef MUTEX_H
#define MUTEX_H

#include "base/Copyable.h"
#include "base/CurrentThread.h"

#include <assert.h>
#include <pthread.h>

namespace ouge {

class MutexLock : NonCopyable {
 public:
  MutexLock() : holder_(0) { pthread_mutex_init(&mutex_, NULL); }
  ~MutexLock() {
    assert(holder_ == 0);
    pthread_mutex_destroy(&mutex_);
  }

  void assertLocked() const { assert(isLockedByThisThread()); }
  bool isLockedByThisThread() const { return holder_ == CurrentThread::tid(); }

  void lock() {
    pthread_mutex_lock(&mutex_);
    assignHolder();
  }
  void unlock() {
    unassignHolder();
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t *getPthreadMutex() { return &mutex_; }

 private:
  friend class Condition;

  // RAII: Condition use this class because Condition will unlock the Mutex
  class UnassignGuard : NonCopyable {
   public:
    UnassignGuard(MutexLock &owner) : owner_(owner) { owner_.unassignHolder(); }
    ~UnassignGuard() { owner_.assignHolder(); }

   private:
    MutexLock &owner_;
  };

  void unassignHolder() { holder_ = 0; }
  void assignHolder() { holder_ = CurrentThread::tid(); }
  pthread_mutex_t mutex_;
  pid_t holder_;
};

// RAII: lock and unlock Mutex object
class MutexLockGuard : NonCopyable {
 public:
  explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) { mutex_.lock(); }

  ~MutexLockGuard() { mutex_.unlock(); }

 private:
  MutexLock &mutex_;
};
}

#endif /* MUTEX_H */
