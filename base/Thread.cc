// TODO: 移除pthread.h
#include "base/Thread.h"
#include "base/CurrentThread.h"
#include "base/Exception.h"
#include "base/Timestamp.h"

#include <sys/prctl.h>
#include <sys/syscall.h>
#include <iostream>
#include <type_traits>
#include <memory>

using namespace std;

namespace ouge {

namespace CurrentThread {

thread_local int         t_cachedTid       = 0;
thread_local char        t_tidString[32]   = {0};
thread_local int         t_tidStringLength = 6;
thread_local const char* t_threadName      = "unknown";

static_assert(is_same_v<int, pid_t>);
}

namespace detail {

pid_t
gettid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void
afterFork() {
    ouge::CurrentThread::t_cachedTid  = 0;
    ouge::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
}

class ThreadNameInitializer {
  public:
    ThreadNameInitializer() {
        ouge::CurrentThread::t_threadName = "main";
        CurrentThread::tid();
        // fork后，子进程的main线程会执行afterFork。
        pthread_atfork(NULL, NULL, &afterFork);
    }
};

// 只有主线程会创建init
ThreadNameInitializer init;

// 线程数据，封装了线程主函数、线程名和tid
struct ThreadData {
    using ThreadFunc = ouge::Thread::ThreadFunc;
    ThreadFunc      func_;
    string          name_;
    weak_ptr<pid_t> wkTid_;

    ThreadData(const ThreadFunc&        func,
               const string&            name,
               const shared_ptr<pid_t>& tid)
            : func_(func), name_(name), wkTid_(tid) {}

    void runInThread() {
        pid_t tid = ouge::CurrentThread::tid();

        shared_ptr<pid_t> ptid = wkTid_.lock();
        if (ptid) {
            *ptid = tid;
            ptid.reset();
        }

        ouge::CurrentThread::t_threadName =
                name_.empty() ? "ougeThread" : name_.c_str();

        // 设置线程名
        ::prctl(PR_SET_NAME, ouge::CurrentThread::t_threadName);

        try {
            func_();
            ouge::CurrentThread::t_threadName = "finished";
        } catch (const Exception& ex) {
            ouge::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
            abort();
        } catch (const std::exception& ex) {
            ouge::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        } catch (...) {
            ouge::CurrentThread::t_threadName = "crashed";
            fprintf(stderr,
                    "unknown exception caught in Thread %s\n",
                    name_.c_str());
            throw;    // rethrow
        }
    }
};

void*
startThread(void* obj) {
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}
}    // namespace ouge::detail
}    // namespace ouge

using namespace ouge;

void
CurrentThread::cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = detail::gettid();
        t_tidStringLength =
                snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
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

atomic_int32_t Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const string& n)
        : started_(false),
          joined_(false),
          pthreadId_(0),
          tid_(new pid_t(0)),
          func_(func),
          name_(n) {
    setDefaultName();
}

Thread::Thread(ThreadFunc&& func, const string& n)
        : started_(false),
          joined_(false),
          pthreadId_(0),
          tid_(new pid_t(0)),
          func_(std::move(func)),
          name_(n) {
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthreadId_);
    }
}

void
Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void
Thread::start() {
    assert(!started_);
    started_ = true;
    // FIXME: move(func_)
    detail::ThreadData* data = new detail::ThreadData(func_, name_, tid_);
    if (pthread_create(&pthreadId_, NULL, &detail::startThread, data)) {
        started_ = false;
        delete data;    // or no delete?
        cerr << "Failed in pthread_create";
    }
}

int
Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}