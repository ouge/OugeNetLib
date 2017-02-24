#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/noncopyable.hpp>

namespace ouge
{

class CountDownLatch : boost::noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};

}

#endif /* COUNTDOWNLATCH_H */
