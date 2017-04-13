#ifndef THREAD_H
#define THREAD_H

#include "Atomic.h"

#include <pthread.h>
#include "base/Copyable.h"
#include <cassert>
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

  void start();
  int join();
  bool started() const { return started_; }
  pid_t tid() const { return *tid_; };
  const std::string& name() const { return name_; };

 private:
  void setDefaultName();
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  std::shared_ptr<pid_t> tid_;
  ThreadFunc func_;
  std::string name_;
  static AtomicInt32 numCreated_;
};
}

#endif /* THREAD_H */
