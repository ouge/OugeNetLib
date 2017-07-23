#include "net/InetAddress.h"

#include "net/Endian.h"
#include "net/SocketsOps.h"

#include <netdb.h>
#include <cstring>
#include <netinet/in.h>
#include <cassert>
#include <iostream>

#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny      = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace std;
using namespace ouge;
using namespace ouge::net;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in));
static_assert(offsetof(sockaddr_in, sin_family) == 0);
static_assert(offsetof(sockaddr_in, sin_port) == 2);

InetAddress::InetAddress(uint16_t port, bool loopbackOnly) {
    static_assert(offsetof(InetAddress, addr_) == 0);
    memset(&addr_, 0, sizeof addr_);
    addr_.sin_family      = AF_INET;
    in_addr_t ip          = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    addr_.sin_port        = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(StringArg ip, uint16_t port) {
    memset(&addr_, 0, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
}

string InetAddress::toIpPort() const {
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

string InetAddress::toIp() const {
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint32_t InetAddress::ipNetEndian() const {
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const {
    return sockets::networkToHost16(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out) {
    assert(out != NULL);
    struct hostent  hent;
    struct hostent* he     = NULL;
    int             herrno = 0;
    memset(&hent, 0, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer,
                              sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) { cerr << "InetAddress::resolve" << endl; }
        return false;
    }
}
