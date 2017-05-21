#ifndef BASE_SINGLETON_H
#define BASE_SINGLETON_H

#include "base/Copyable.h"

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

namespace ouge {

namespace detail {

// 判断一个类型有没有no_destroy成员
template <typename T>
struct has_no_destroy {
    template <typename C>
    static char test(decltype(&C::no_destroy));
    template <typename C>
    static int32_t test(...);

    // 如果value为true，说明调用了test(decltype(&C::no_destroy)
    // sizeof（函数） 不需要函数实现，只需要函数返回值。
    const static bool value = sizeof(test<T>(0)) == 1;
};
}    // ouge::detail

template <typename T>
class Singleton : NonCopyable {
  public:
    static T& instance() {
        pthread_once(&ponce_, &Singleton::init);
        assert(value_ != NULL);
        return *value_;
    }

  private:
    Singleton();
    ~Singleton();

    static void init() {
        value_ = new T();
        if (!detail::has_no_destroy<T>::value) {
            ::atexit(destroy);
        }
    }

    static void destroy() {
        using T_must_be_complete_type = char[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void)dummy;
        delete value_;
        value_ = NULL;
    }

  private:
    static pthread_once_t ponce_;
    static T*             value_;
};

template <typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::value_ = NULL;
}    // ouge

#endif /* SINGLETON_H */
