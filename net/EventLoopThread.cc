#include "net/EventLoopThread.h"
#include "net/EventLoop.h"

#include <functional>
#include <cassert>
#include <memory>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace ouge;
using namespace ouge::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string&        name)
        : loop_(NULL), exiting_(false), callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (threadPtr_ != NULL && loop_ != NULL) {
        loop_->quit();
        threadPtr_->join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    loop_->assertInLoopThread();
    assert(!threadPtr_);
    threadPtr_.reset(new thread(bind(&EventLoopThread::threadFunc, this)));
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return loop_ != NULL; });
    }

    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    if (callback_) { callback_(&loop); }

    {
        unique_lock<mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();

    loop_ = NULL;
}
