#include "net/EventLoopThread.h"
#include "net/EventLoop.h"

#include <functional>

using namespace ouge;
using namespace ouge::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string&        name)
        : loop_(NULL),
          exiting_(false),
          thread_(std::bind(&EventLoopThread::threadFunc, this), name),
          mutex_(),
          cond_(),
          callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_
        != NULL)    // not 100% race-free, eg. threadFunc could be running callback_.
    {
        // still a tiny chance to call destructed object, if threadFunc exits just now.
        // but when EventLoopThread destructs, usually programming is exiting anyway.
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cond_.wait(lock, loop_ != NULL);
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    if (callback_) { callback_(&loop); }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    //assert(exiting_);
    loop_ = NULL;
}
