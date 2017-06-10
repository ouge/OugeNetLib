#ifndef BASE_COPYABLE_H
#define BASE_COPYABLE_H

namespace ouge {

// 可复制基类
class Copyable {};

// 不可复制基类
class NonCopyable {
  public:
    // 需要显式设置默认无参构造函数，否则不会合成。
    NonCopyable()                   = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&)      = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable& operator=(NonCopyable&&) = delete;
};
}    // namespace ouge

#endif    // BASE_COPYABLE_H
