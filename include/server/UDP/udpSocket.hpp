#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <muduo/base/noncopyable.h>
#include <string>
#include <netinet/in.h>

namespace muduo{
    namespace net{
        class InetAddress;
    }
}

// 简单的UDP socket类 仅供心跳使用
class udpSocket{
public:
    explicit udpSocket(int sockfd);
    ~udpSocket();

    int fd() const { return sockfd_; }

    void bindAddress(const muduo::net::InetAddress& localaddr);
    void setReuseAddr(bool on);
    void setReusePort(bool on);

    ssize_t recvFrom(void* buf, size_t len, struct sockaddr_in* addr);
    ssize_t sendTo(const void* buf, size_t len, const struct sockaddr_in& addr);

private:
    const int sockfd_;

};


#endif