#ifndef BASE_ASYNCLOGGING_H
#define BASE_ASYNCLOGGING_H

#include "base/Copyable.h"
#include "base/CountDownLatch.h"
#include "base/LogStream.h"
#include "base/Thread.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace ouge {

class AsyncLogging : NonCopyable {
  public:
    AsyncLogging(const std::string& basename,
                 size_t             rollSize,
                 int                flushInterval = 3);
    ~AsyncLogging() {
        if (running_) {
            stop();
        }
    }
    void append(const char* logline, int len);
    void start();
    void stop();

  private:
    AsyncLogging(const AsyncLogging&);
    void operator=(const AsyncLogging&);
    void threadFunc();

    using Buffer       = ouge::detail::FixedBuffer<ouge::detail::kLargeBuffer>;
    using BufferVector = boost::ptr_vector<Buffer>;
    using BufferPtr    = BufferVector::auto_type;

    const int               flushInterval_;
    bool                    running_;
    std::string             basename_;
    size_t                  rollSize_;
    ouge::Thread            thread_;
    ouge::CountDownLatch    latch_;
    std::mutex              mutex_;
    std::condition_variable cond_;
    BufferPtr               currentBuffer_;
    BufferPtr               nextBuffer_;
    BufferVector            buffers_;
};
}    // namespace ouge

#endif    // BASE_ASYNCLOGGING_H
