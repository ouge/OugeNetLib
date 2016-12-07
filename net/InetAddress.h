#ifndef NET_INETADDRESS_H
#define NET_INETADDRESS_H

#include <netinet/in.h>
#include <string>

using namespace std;

class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
//    InetAddress(StringArg ip, uint16_t port);
    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_(addr) { }
    
    sa_family_t family() const { return addr_.sin_family; }
    string toIp() const;
    string toIpPort() const;
    uint16_t toPort() const;

    const struct sockaddr_in* getSockAddr() const { return &addr_; }
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr;}

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const { return addr_.sin_port; }

    static bool resolve(string hostname, InetAddress* result);

private:
    struct sockaddr_in addr_;
};


#endif