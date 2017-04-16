#ifndef BASE_THREADLOCALSINGLETON_H
#define BASE_THREADLOCALSINGLETON_H

#include "base/Copyable.h"

#include <assert.h>
#include <pthread.h>

namespace ouge {

// TODO:使用？
// 线程局部的单例类型
template <typename T>
class ThreadLocalSingleton : NonCopyable {
 public:
  static T& instance() {
    if (!t_value_) {
      t_value_ = new T();
      // 将t_value_与Deleter::pkey_关联
      deleter_.set(t_value_);
    }
    return *t_value_;
  }

  static T* pointer() { return t_value_; }

 private:
  ThreadLocalSingleton();
  ~ThreadLocalSingleton();

  // 释放线程局部变量的空间
  static void destructor(void* obj) {
    assert(obj == t_value_);
    using T_must_be_complete_type = char[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;
    delete t_value_;
    t_value_ = 0;
  }

  class Deleter {
   public:
    Deleter() {
      // 创建一个键
      // 线程正常退出时，会调用 ThreadLocalSingleton::destructor(t_value_)
      ::pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
    }

    ~Deleter() { ::pthread_key_delete(pkey_); }

    void set(T* newObj) {
      assert(pthread_getspecific(pkey_) == nullptr);
      // 将newObj与pkey_关联
      ::pthread_setspecific(pkey_, newObj);
    }

    pthread_key_t pkey_;
  };

  // 线程局部变量
  static __thread T* t_value_;
  static Deleter deleter_;
};

// __thread变量只能初始化为编译期常量
template <typename T>
__thread T* ThreadLocalSingleton<T>::t_value_ = nullptr;

// 嵌套依赖类型 ThreadLocalSingleton<T>::Deleter 前面需要加上 typename
template <typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;
}

#endif