// TODO: 移除pthread.h
#include "base/Thread.h"
#include "base/CurrentThread.h"
#include "base/Exception.h"
#include "base/Logging.h"
#include "base/Timestamp.h"

#include <sys/prctl.h>
#include <sys/syscall.h>

#include <type_traits>
#include <memory>

using namespace std;

namespace ouge {


namespace detail {



struct ThreadData {
    typedef ouge::Thread::ThreadFunc ThreadFunc;
    ThreadFunc                       func_;
    string                           name_;
    weak_ptr<pid_t>                  wkTid_;

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

std::atomic<int> Thread::numCreated_;

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
        LOG_SYSFATAL << "Failed in pthread_create";
    }
}

int
Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}