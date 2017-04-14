#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <pthread.h>
#include <stdint.h>

namespace ouge::CurrentThread {

// __thread 变量是线程私有的
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();

inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char* tidString() { return t_tidString; }

inline int tidStringLength() { return t_tidStringLength; }

inline const char* name() { return t_threadName; }

bool isMainThread();

void sleepUsec(int64_t usec);
}  // ouge::CurrentThread

#endif /* CURRENTTHREAD_H */
