#include <assert.h>
#include <poll.h>
#include "EventLoop.h"

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop() : 
    looping_(false), 
    threadId_(CurrentThread::tid()) 
{

  //LOG_TRACE << "EventLoop created " << this 
  //   << " in thread " << threadId_;

  if (t_loopInThisThread) {
    //LOG_FATAL << "Another EventLoop " << t_loopInThisThread
    //    << " exits in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  
}

EventLoop::~EventLoop()
{
  assert(!looping_);
  t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

void EventLoop::assertInLoopThread()
{
  if (!isinLoopThread()) 
  {
    abortNotInLoopThread();
  }
}


void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;

  poll(NULL, 0, 5 * 1000);

  // LOG_TRACE
  looping_ = false;

}

