#ifndef BASE_CURRENTTHREAD_H
#define BASE_CURRENTTHREAD_H

#include "base/Copyable.h"

#include <cstdint>

namespace ouge {

class CurrentThread : NonCopyable {
  public:
    CurrentThread() {}

    void cacheTid();
    int  tid() {
        if (cachedTid_ == 0) {
            cacheTid();
        }
        return cachedTid_;
    }

    const char* tidString() const { return tidString_; }

    int tidStringLength() const { return tidStringLength_; }

    const char* name() const { return threadName_; }

    bool isMainThread() const;

    void sleepUsec(int64_t usec);

  private:
    int         cachedTid_       = 0;
    char        tidString_[32]   = {0};
    int         tidStringLength_ = 6;
    const char* threadName_      = "unknown";
};

extern thread_local CurrentThread t_currentThread;

namespace detail {}

}    // namespace ouge

#endif /* BASE_CURRENTTHREAD_H */
