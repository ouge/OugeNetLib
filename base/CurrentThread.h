#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

#include <stdint.h>

namespace ouge::CurrentThread {

extern thread_local int         t_cachedTid;
extern thread_local char        t_tidString[32];
extern thread_local int         t_tidStringLength;
extern thread_local const char* t_threadName;

void cacheTid();

inline int
tid() {
    if (t_cachedTid == 0) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char*
tidString() {
    return t_tidString;
}

inline int
tidStringLength() {
    return t_tidStringLength;
}

inline const char*
name() {
    return t_threadName;
}

bool isMainThread();

void sleepUsec(int64_t usec);

}    // ouge::CurrentThread

#endif /* CURRENTTHREAD_H */
