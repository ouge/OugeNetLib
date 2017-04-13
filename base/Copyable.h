#ifndef BASE_COPYABLE_H
#define BASE_COPYABLE_H

namespace ouge {
class Copyable {};

class NonCopyable {
 public:
  NonCopyable() {}
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};
}

#endif
