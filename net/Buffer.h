#ifndef BUFFER_H
#define BUFFER_H

#include "../base/StringPiece.h"
#include "../base/copyable.h"

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

namespace ouge {
namespace net {

class Buffer : public ouge::copyable {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize);
  void swap(Buffer& rhs);
  size_t readableBytes() const;
  size_t writableBytes() const;
  size_t prependableBytes() const;
  const char* peek() const;
  const char* findCRLF() const;
  const char* findCRLF(const char*) const;
  const char* findEOL() const;
  const char* findEOL(const char*) const;
  void retrieve(size_t);
  void retrieveUntil(const char*);
  void retrieveInt64();
  void retrieveInt32();
  void retrieveInt16();
  void retrieveInt8();
  void retrieveAll();
  std::string retrieveAllAsString();
  std::string retrieveAsString(size_t);
  StringPiece toStringPiece() const;
  void append(const StringPiece* str);
  void append(const char*, size_t);
  void append(const void*, size_t);
  void ensureWritableBytes(size_t);
  char* beginWrite();
  const char* beginWrite() const;
  void hasWritten(size_t);
  void unwrite(size_t len);
  void appendInt64(int64_t);
  void appendInt32(int32_t);
  void appendInt16(int16_t);
  void appendInt8(int8_t);
  void prependInt64(int64_t);
  void prepend(const void*, size_t len);
  void shrink(size_t reserve);
  size_t internalCapacity() const;
  ssize_t readFd(int, int*);

 private:
  char* begin();
  const char* begin() const;
  void makeSpace(size_t len);

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

  static const char kCRLF[];
};
}
}

#endif /* BUFFER_H */
