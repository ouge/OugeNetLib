#ifndef BASE_CONDITION_H
#define BASE_CONDITION_H

#include "base/Copyable.h"
#include "base/Mutex.h"

#include <pthread.h>
#include <cerrno>

namespace ouge {

// 条件变量
class Condition : NonCopyable {
  public:
    explicit Condition(MutexLock& mutex) : mutex_(mutex) {
        ::pthread_cond_init(&pcond_, NULL);
    }
    ~Condition() { ::pthread_cond_destroy(&pcond_); }

    // 在mutex_上锁的时候才能调用 wait()
    // 使用时需要把 bool 条件和 wait() 放到 while
    // 循环中（避免惊群现象、虚假唤醒）
    // {
    //   MutexLockGuard lock(mutex);
    //   while (判断条件) {
    //     cond.wait();
    //   }
    //   condition ok
    // }
    void wait() {
        MutexLock::UnassignGuard ug(mutex_);
        // pthread_cond_wait() 会对 mutex_ 先释放锁，再将当前线程挂起。
        ::pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
        // 在该处线程被唤醒，且重新获得锁
        // 退出wait()函数时，ug 析构会重新将mutex_的拥有者设置为当前线程
    }

    // 带超时判断的wait
    bool waitForSeconds(double seconds) {
        struct timespec abstime;
        ::clock_gettime(CLOCK_REALTIME, &abstime);
        const int64_t kNanoSecondsPerSecond = 1e9;
        int64_t       nanoseconds =
                static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds)
                                              / kNanoSecondsPerSecond);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds)
                                            % kNanoSecondsPerSecond);
        MutexLock::UnassignGuard ug(mutex_);
        return ETIMEDOUT == ::pthread_cond_timedwait(&pcond_,
                                                     mutex_.getPthreadMutex(),
                                                     &abstime);
    }

    // {
    //   {
    //       MutexLockGuard lock(mutex);
    //       修改条件；
    //   }
    //   cond.notify();
    // }
    void notify() { ::pthread_cond_signal(&pcond_); }

    void notifyAll() { ::pthread_cond_broadcast(&pcond_); }

  private:
    MutexLock&     mutex_;
    pthread_cond_t pcond_;
};
}

#endif /* CONDITION_H */
