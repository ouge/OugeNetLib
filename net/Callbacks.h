#ifndef NET_CALLBACKS_H
#define NET_CALLBACKS_H

#include <functional>
#include <memory>

#include "base/Timestamp.h"
#include "base/Types.h"

namespace ouge {

template <typename To, typename From>
inline std::shared_ptr<To> down_pointer_cast(const std::shared_ptr<From>& f) {
  assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
  return std::static_pointer_cast<To>(f);
}

namespace net {

class Buffer;
class TcpConnection;

// 暴露给用户的回调函数接口
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer,
                            Timestamp receiveTime);
}
}

#endif  // MUDUO_NET_CALLBACKS_H
