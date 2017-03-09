#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "StringPiece.h"
#include "Types.h"

#include <boost/noncopyable.hpp>
#include <cassert>
#include <cstring>
#include <string>

namespace ouge {

namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : boost::noncopyable {};
}

class LogStream : boost::noncopyable {
  typedef LogStream self;

 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
  self& operator<<(bool);
  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);
  self& operator<<(const void*);
  self& operator<<(float);
  self& operator<<(double);
  self& operator<<(char);
  self& operator<<(const char*);
  self& operator<<(const unsigned char*);
  self& operator<<(const std::string&);
  self& operator<<(const StringPiece&);
  self& operator<<(const Buffer&);
  void append(const char*, int);
  const Buffer& buffer() const;
  void resetBuffer();

 private:
  void staticCheck();
  template <typename T>
  void formatInteger(T);
  Buffer buffer_;
  static const int kMaxNumericSize = 32;
};

class Fmt {
 public:
  template <typename T>
  Fmt(const char* fmt, T val);
  const char* date() const;
  int length() const;

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt);
}

#endif /* LOGSTREAM_H */
