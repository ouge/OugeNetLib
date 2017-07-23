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
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

    InetAddress(StringArg ip, uint16_t port);

    explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

    sa_family_t family() const { return addr_.sin_family; }
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t    toPort() const;

    const struct sockaddr* getSockAddr() const {
        return sockets::sockaddr_cast(&addr_);
    }

    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const { return addr_.sin_port; }

    static bool resolve(StringArg hostname, InetAddress* result);

  private:
    struct sockaddr_in addr_;
};
}
}

#endif    // NET_INETADDRESS_H
