#ifndef BASE_WEAKCALLBACK_H
#define BASE_WEAKCALLBACK_H

#include <functional>
#include <memory>
#include <iostream>

namespace ouge {

// 弱回调仿函数，针对参数为weak_ptr的实际回调函数的封装。
// 会先对weak_ptr判断对象是否存在。
template <typename CLASS, typename... ARGS>
class WeakCallback {
  public:
    WeakCallback(const std::weak_ptr<CLASS>& object,
                 const std::function<void(CLASS*, ARGS...)>& function)
            : object_(object), function_(function) {}

    void operator()(ARGS&&... args) const {
        std::shared_ptr<CLASS> ptr(object_.lock());

        if (ptr) {
            // 如果weak_ptr的lock操作成功，说明对象生存，执行回调
            function_(ptr.get(), std::forward<ARGS>(args)...);
        } else {
            std::cerr << "expired" << std::endl;
        }
    }

  private:
    std::weak_ptr<CLASS> object_;
    std::function<void(CLASS*, ARGS...)> function_;
};

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...>
makeWeakCallback(const std::shared_ptr<CLASS>& object,
                 void (CLASS::*function)(ARGS...)) {
    return WeakCallback<CLASS, ARGS...>(object, function);
}

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...>
makeWeakCallback(const std::shared_ptr<CLASS>& object,
                 void (CLASS::*function)(ARGS...) const) {
    return WeakCallback<CLASS, ARGS...>(object, function);
}
}    // namespace ouge

#endif    // BASE_WEAKCALLBACK_H
