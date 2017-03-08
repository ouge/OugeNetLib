#ifndef MUTEX_H
#define MUTEX_H

#include "CurrentThread.h"

#include <pthread.h>
#include <boost/noncopyable.hpp>
#include <cassert>

namespace ouge {

class MutexLock : boost::noncopyable {
 public:
  MutexLock() : holder_(0) { pthread_mutex_init(&mutex_, NULL); }
  ~MutexLock() {
    assert(holder_ == 0);
    pthread_mutex_destroy(&mutex_);
  }

  bool isLockedByThisThread() { return holder_ == CurrentThread::tid(); }
  void assertLocked() { assert(isLockedByThisThread()); }
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
  class UnassignGuard : boost::noncopyable {
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

class MutexLockGuard : boost::noncopyable {
 public:
  explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) { mutex_.lock(); }

  ~MutexLockGuard() { mutex_.unlock(); }

 private:
  MutexLock &mutex_;
};

// 防止出现 MutexLockGuard(mutex_) 类似的误用
#define MutexLockGuard(x) error "Missing guard object name"
}

#endif /* MUTEX_H */
