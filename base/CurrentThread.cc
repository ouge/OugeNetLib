#include "base/CurrentThread.h"

#include <type_traits>
#include <cstdio>
#include <thread>
#include <strstream>

// namespace ouge {
// namespace detail {

// pid_t
// gettid() {
//     return static_cast<pid_t>(::syscall(SYS_gettid));
// }

// void
// afterFork() {
//     ouge::CurrentThread::t_cachedTid  = 0;
//     ouge::CurrentThread::t_threadName = "main";
//     CurrentThread::tid();
//     // no need to call pthread_atfork(NULL, NULL, &afterFork);
// }

// class ThreadNameInitializer {
//   public:
//     ThreadNameInitializer() {
//         ouge::CurrentThread::t_threadName = "main";
//         CurrentThread::tid();
//         pthread_atfork(NULL, NULL, &afterFork);
//     }
// };

// ThreadNameInitializer init;

// }}

using namespace ouge;

thread_local CurrentThread t_currentThread;

void
CurrentThread::cacheTid() {
    if (cachedTid_ == 0) {
        std::strstream stream;
        stream << std::this_thread::get_id();

        stream >> tidString_;

        cachedTid_ = detail::gettid();
        tidStringLength_ =
                snprintf(tidString_, sizeof tidString_, "%5d ", cachedTid_);
    }
}

bool
CurrentThread::isMainThread() {
    return tid() == ::getpid();
}

void
CurrentThread::sleepUsec(int64_t usec) {
    struct timespec ts = {0, 0};
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec =
            static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
    ::nanosleep(&ts, NULL);
}
