#ifndef NET_TIMERQUEUE_H
#define NET_TIMERQUEUE_H

#include "base/Mutex.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"
#include "net/Channel.h"
#include "base/Copyable.h"

#include <set>
#include <vector>

namespace ouge {
namespace net {

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : NonCopyable {
  public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads
    TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);
    TimerId addTimer(TimerCallback&& cb, Timestamp when, double interval);
    void cancel(TimerId timerId);

  private:
    // FIXME: use unique_ptr<Timer> instead of raw pointers.
    using Entry          = std::pair<Timestamp, Timer*>;
    using TimerList      = std::set<Entry>;
    using ActiveTimer    = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;
    const int  timerfd_;
    Channel    timerfdChannel_;
    TimerList  timers_;    // Timer list sorted by expiration

    // for cancel()
    ActiveTimerSet activeTimers_;
    bool           callingExpiredTimers_; /* atomic */
    ActiveTimerSet cancelingTimers_;
};
}    // namespace ouge::net
}    // namespace ouge
#endif
