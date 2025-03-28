#include "udpSocket.hpp"
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

using namespace muduo;
using namespace muduo::net;

udpSocket::udpSocket(int sockfd) : sockfd_(sockfd){

}

udpSocket::~udpSocket(){
    close(sockfd_);
}

void udpSocket::bindAddress(const muduo::net::InetAddress& localaddr){
    // 使用强制类型转换
    const struct sockaddr_in* addr_ptr = reinterpret_cast<const struct sockaddr_in*>(localaddr.getSockAddr());
    int ret = bind(sockfd_, (struct sockaddr*)addr_ptr, sizeof(*addr_ptr));
    if (ret < 0) {
        LOG_FATAL << "UdpSocket::bindAddress";
    }
}

void udpSocket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, 
                &optval, static_cast<socklen_t>(sizeof optval));
}

void udpSocket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                &optval, static_cast<socklen_t>(sizeof optval));
}

ssize_t udpSocket::recvFrom(void* buf, size_t len, struct sockaddr_in* addr){
    socklen_t addrlen = sizeof(*addr);
    return recvfrom(sockfd_, buf, len, 0, 
                     (struct sockaddr*)addr, &addrlen);
}

ssize_t udpSocket::sendTo(const void* buf, size_t len, const struct sockaddr_in& addr) {
    return sendto(sockfd_, buf, len, 0, 
                   (struct sockaddr*)&addr, sizeof(addr));
}
