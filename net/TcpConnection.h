#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

namespace ouge {
namespace net {

class TcpConnection : boost::noncopyable,
                      publi boost::enable_shared_from_this<TcpConnection> {
 public:
  void send(const void* message, size_t len);
  void send(StringPiece& message);
  void send(Buffer* message);
};
}
}
#endif /* TCPCONNECTION_H */
