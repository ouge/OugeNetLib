#ifndef NET_INETADDRESS_H
#define NET_INETADDRESS_H

#include "base/Copyable.h"
#include "base/StringPiece.h"

#include <netinet/in.h>

namespace ouge {
namespace net {
namespace sockets {
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
}

///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress : public Copyable {
  public:
    /// Constructs an endpoint with given port number.
    /// Mostly used in TcpServer listening.
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    /// Constructs an endpoint with given ip and port.
    /// @c ip should be "1.2.3.4"
    InetAddress(StringArg ip, uint16_t port);

    /// Constructs an endpoint with given struct @c sockaddr_in
    /// Mostly used when accepting new connections
    explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

    sa_family_t family() const { return addr_.sin_family; }
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t    toPort() const;

    // default copy/assignment are Okay

    const struct sockaddr* getSockAddr() const {
        return sockets::sockaddr_cast(&addr_);
    }

    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const { return addr_.sin_port; }

    // resolve hostname to IP address, not changing port or sin_family
    // return true on success.
    // thread safe
    static bool resolve(StringArg hostname, InetAddress* result);
    // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t
    // port = 0);

  private:
    struct sockaddr_in addr_;
};
}
}

#endif    // NET_INETADDRESS_H
