#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include "base/Atomic.h"
#include "base/Copyable.h"

#include <assert.h>
#include <pthread.h>
#include <functional>
#include <memory>

namespace ouge {

class Thread : NonCopyable {
 public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(const ThreadFunc& func,
                  const std::string& name = std::string());
  explicit Thread(ThreadFunc&& func, const std::string& name = std::string());
  ~Thread();

  // 启动线程执行函数 func_
  void start();

  // 将线程置于分离状态
  int join();

  bool started() const { return started_; }
  pid_t tid() const { return *tid_; };
  const std::string& name() const { return name_; };

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();
  bool started_;  // func_是否已经开始执行
  bool joined_;   // 线程是否处于分离状态
  pthread_t pthreadId_;
  std::shared_ptr<pid_t> tid_;
  ThreadFunc func_;  // 线程执行函数
  std::string name_;
  static AtomicInt32 numCreated_;  // 当前线程总数
};
}

#endif /* THREAD_H */
