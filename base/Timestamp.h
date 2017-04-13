#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "base/Copyable.h"

#include <boost/operators.hpp>
#include <cstdint>
#include <string>

namespace ouge {

class Timestamp : public Copyable,
                  public boost::less_than_comparable<Timestamp> {
 public:
  Timestamp();
  explicit Timestamp(int64_t);

  void swap(Timestamp&);
  std::string toString() const;
  std::string toFormattedString(bool) const;
  bool valid() const;
  int64_t microSecondsSinceEpoch() const;
  static Timestamp now();
  static Timestamp invalid();
  static Timestamp fromUnixTime(time_t);
  static Timestamp fromUnixTime(time_t, int);
  static const int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp, Timestamp);
inline bool operator==(Timestamp, Timestamp);
inline double timeDifference(Timestamp, Timestamp);
inline Timestamp addTime(Timestamp, double);
}

#endif /* TIMESTAMP_H */
