#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <string>

namespace ouge {

template <typename To, typename From>
inline To
implicit_cast(From const& f) {
    return f;
}

template <typename To, typename From>
inline To
down_cast(From* f) {
    // TODO: 判断是否允许向下转换
    if (false) {
        implicit_cast<From*, To>(0);
    }
    return static_cast<To>(f);
}
}    // namespace ouge

#endif    // BASE_TYPES_H
