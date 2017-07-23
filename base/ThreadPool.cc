#include "base/ThreadPool.h"

#include "base/Exception.h"

#include <cassert>
#include <cstdio>
#include <functional>
#include <thread>

using namespace std;
using namespace ouge;

ThreadPool::ThreadPool(const std::string& nameArg)
        : mutex_(),
          notEmpty_(),
          notFull_(),
          name_(nameArg),
          maxQueueSize_(0),
          running_(false) {}

ThreadPool::~ThreadPool() {
    if (running_) { stop(); }
}

void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        char id[32];
        snprintf(id, sizeof id, "%d", i + 1);
        threads_.push_back(new thread([this]() { runInThread(); }));
    }
    if (numThreads == 0 && threadInitCallback_) { threadInitCallback_(); }
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
        notEmpty_.notify_all();
    }
    for_each(threads_.begin(), threads_.end(),
             std::bind(&thread::join, placeholders::_1));
}

size_t ThreadPool::queueSize() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(const Task& task) {
    if (threads_.empty()) {
        task();
    } else {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this] { return !isFull(); });
        assert(!isFull());

        queue_.push_back(task);
        notEmpty_.notify_one();
    }
}

void ThreadPool::run(Task&& task) {
    if (threads_.empty()) {
        task();
    } else {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [this] { return !isFull(); });
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify_one();
    }
}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock<std::mutex> lock(mutex_);
    notEmpty_.wait(lock, [this] { return !queue_.empty() || !running_; });
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0) { notFull_.notify_one(); }
    }
    return task;
}

bool ThreadPool::isFull() const {
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() {
    try {
        if (threadInitCallback_) { threadInitCallback_(); }
        while (running_) {
            Task task(move(take()));
            if (task) { task(); }
        }
    } catch (const Exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    } catch (const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    } catch (...) {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n",
                name_.c_str());
        throw;    // rethrow
    }
}
