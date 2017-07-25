#include "net/TcpConnection.h"

#include "base/WeakCallback.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/Socket.h"
#include "net/SocketsOps.h"
#include "net/TimerId.h"

#include <functional>
#include <cerrno>
#include <iostream>

using namespace std;
using namespace ouge;
using namespace ouge::net;

void ouge::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
    cout << conn->localAddress().toIpPort() << " -> "
         << conn->peerAddress().toIpPort() << " is "
         << (conn->connected() ? "UP" : "DOWN") << endl;
}

void ouge::net::defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf,
                                       Timestamp) {
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& nameArg,
                             int sockfd, const InetAddress& localAddr,
                             const InetAddress& peerAddr)
        : loop_(loop),
          name_(nameArg),
          state_(kConnecting),
          reading_(true),
          socket_(new Socket(sockfd)),
          channel_(new Channel(loop, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr),
          highWaterMark_(64 * 1024 * 1024) {
    channel_->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    cout << "TcpConnection::ctor[" << name_ << "] at " << this
         << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    cout << "TcpConnection::dtor[" << name_ << "] at " << this
         << " fd=" << channel_->fd() << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const {
    return socket_->getTcpInfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const void* data, int len) {
    send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            // loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,
            //                            this,    // FIXME
            //                            message.as_string()));
            std::shared_ptr<TcpConnection> temp(shared_from_this());
            loop_->runInLoop([temp, message]() {
                temp->sendInLoop(message.as_string());
            });

            // std::forward<std::string>(message)));
        }
    }
}

// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            // loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,
            //                            this,    // FIXME
            //                            buf->retrieveAllAsString()));
            // std::forward<std::string>(message)));
            std::shared_ptr<TcpConnection> temp(shared_from_this());
            loop_->runInLoop([temp, buf]() {
                temp->sendInLoop(buf->retrieveAllAsString());
            });
        }
    }
}

void TcpConnection::sendInLoop(const StringPiece& message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote     = 0;
    size_t  remaining  = len;
    bool    faultError = false;
    if (state_ == kDisconnected) {
        cerr << "disconnected, give up writing" << endl;
        return;
    }

    // 如果输出队列中没有东西需要发送，直接发送不需要先存入buffer
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(
                        bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                cerr << "TcpConnection::sendInLoop" << endl;
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(bind(highWaterMarkCallback_, shared_from_this(),
                                    oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote,
                             remaining);
        if (!channel_->isWriting()) { channel_->enableWriting(); }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(
                bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        // FIXME: 可能出现问题
        setState(kDisconnecting);
        loop_->queueInLoop(
                bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->runAfter(seconds, makeWeakCallback(shared_from_this(),
                                                  &TcpConnection::forceClose));
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting) {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

const char* TcpConnection::stateToString() const {
    switch (state_) {
        case kDisconnected: return "kDisconnected";
        case kConnecting: return "kConnecting";
        case kConnected: return "kConnected";
        case kDisconnecting: return "kDisconnecting";
        default: return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

void TcpConnection::startRead() {
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading()) {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead() {
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading()) {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    int     savedErrno = 0;
    ssize_t n          = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        cerr << "TcpConnection::handleRead" << endl;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = sockets::write(channel_->fd(), outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,
                                                 shared_from_this()));
                }
                if (state_ == kDisconnecting) { shutdownInLoop(); }
            }
        } else {
            cerr << "TcpConnection::handleWrite";
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    } else {
        cout << "Connection fd = " << channel_->fd()
             << " is down, no more writing" << endl;
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    cout << "fd = " << channel_->fd() << " state = " << stateToString() << endl;
    assert(state_ == kConnected || state_ == kDisconnecting);

    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    // must be the last line
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    cerr << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err
         << " " << ::strerror(err) << endl;
}
