#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include "InetAddress.h"


class Socket
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    { }

    ~Socket();

    int fd() const { return sockfd_; }
    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpInfoString(char* buf, int len) const;

    void bindAddress(const InetAddress& localaddr);
    
    void listen();

    int accept(InetAddress* peeraddr);

    void shutdownWrite();
    
    // Enable/disable TCP_NODELAY
    void setTcpNoDelay(bool on);

    // Enable/disable SO_REUSEADDR
    void setReuseAddr(bool on);

    // Enable/disable SO_REUSEPORT
    void setReusePort(bool on);

    // Enable/disable SO_KEEPALIVE
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};


#endif