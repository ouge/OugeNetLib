#ifndef BASE_COPYABLE_H
#define BASE_COPYABLE_H

namespace ouge {
class Copyable {};

class NonCopyable {
 public:
  // 需要显式设置默认无参构造函数，否则不会合成。
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};
}

#endif
