#ifndef BASE_TIMESTAMP_H
#define BASE_TIMESTAMP_H

#include "base/Copyable.h"

#include <boost/operators.hpp>
#include <cstdint>
#include <string>

namespace ouge {

//  Time stamp in UTC, in microseconds resolution.
//  This class is immutable.
//  It's recommended to pass it by value, since it's passed in register on x64.
//
// less_than_comparable<T> 要求类型T具有以下语义：
// 1) bool operator<(const T&,const T&);
// 2) bool operator>(const T&,const T&);
// 3) bool operator<=(const T&,const T&);
// 4) bool operator>=(const T&,const T&);
//
// 要派生自 boost::less_than_comparable, 派生类(T)必须提供：
// 1) bool operator<(const T&, const T&);
class Timestamp : public Copyable,
                  public boost::less_than_comparable<Timestamp>,
                  public boost::equality_comparable<Timestamp> {
  public:
    // Constucts an invalid Timestamp.
    Timestamp() : microSecondsSinceEpoch_(0) {}

    // Constucts a Timestamp at specific time
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
            : microSecondsSinceEpoch_(microSecondsSinceEpochArg) {}

    void swap(Timestamp& that) {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t  secondsSinceEpoch() const {
        return static_cast<time_t>(microSecondsSinceEpoch_
                                   / kMicroSecondsPerSecond);
    }

    static Timestamp now();

    static Timestamp invalid() { return Timestamp(); }

    static Timestamp fromUnixTime(time_t t) { return fromUnixTime(t, 0); }
    static Timestamp fromUnixTime(time_t t, int microseconds) {
        return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond
                         + microseconds);
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

  private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// Gets time difference of two timestamps, result in seconds.
inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

// Add seconds to given timestamp.
inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta =
            static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}
}    // namespace ouge

#endif /* TIMESTAMP_H */
