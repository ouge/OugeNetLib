#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <pthread.h>
#include <cstdint>

namespace ouge {
// TODO: namespace CurrentThread??
namespace CurrentThread {

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();
inline int tid();
inline const char* tidString();
inline int tidStringLength();
inline const char* name();
bool isMainThread();
void sleepUsec(int64_t usec);
}
}

#endif /* CURRENTTHREAD_H */
