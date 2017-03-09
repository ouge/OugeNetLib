#ifndef THREAD_H
#define THREAD_H

#include "Atomic.h"

#include <pthread.h>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

namespace ouge {

class Thread : boost::noncopyable {
 public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(const ThreadFunc&, const std::string& name = std::string());
  explicit Thread(ThreadFunc&&, const std::string& name = std::string());
  ~Thread();

  void start();
  int join();

  bool started() const { return started_; }
  pid_t tid();
  const std::string& name() const;

 private:
  void setDefalultName();

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
