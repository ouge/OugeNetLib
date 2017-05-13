#ifndef BASE_MUTEX_H
#define BASE_MUTEX_H

#include "base/Copyable.h"
#include "base/CurrentThread.h"

#include <assert.h>
#include <pthread.h>

namespace ouge {

class MutexLock : NonCopyable {
  public:
    MutexLock() : holder_(0) { ::pthread_mutex_init(&mutex_, NULL); }
    ~MutexLock() {
        assert(holder_ == 0);
        ::pthread_mutex_destroy(&mutex_);
    }

    // 判断当前线程是否持有锁
    void assertLocked() const { assert(isLockedByThisThread()); }
    bool isLockedByThisThread() const {
        return holder_ == CurrentThread::tid();
    }

    // 加锁
    void lock() {
        ::pthread_mutex_lock(&mutex_);
        // 将锁的持有者设置为当前线程
        assignHolder();
    }

    // 解锁
    void unlock() {
        // 先将锁的持有者置空
        unassignHolder();
        ::pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *getPthreadMutex() { return &mutex_; }

  private:
    friend class Condition;

    // RAII
    //条件变量Condition使用
    class UnassignGuard : NonCopyable {
      public:
        UnassignGuard(MutexLock &owner) : owner_(owner) {
            // UnassignGuard对象创建时，会将锁的拥有者设置为空
            owner_.unassignHolder();
        }
        ~UnassignGuard() {
            // 析构时重新将锁的拥有者设置成当前线程
            owner_.assignHolder();
        }

      private:
        MutexLock &owner_;
    };

    void            unassignHolder() { holder_ = 0; }
    void            assignHolder() { holder_ = CurrentThread::tid(); }
    pthread_mutex_t mutex_;
    pid_t           holder_;
};

// RAII
// 初始化即对MutexLock加锁，退出作用域时即对MutexLock解锁
class MutexLockGuard : NonCopyable {
  public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex) { mutex_.lock(); }

    ~MutexLockGuard() { mutex_.unlock(); }

  private:
    MutexLock &mutex_;
};
}    // namespace ouge

#endif /* BASE_MUTEX_H */
