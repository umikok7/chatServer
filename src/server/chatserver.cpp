#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
#include <muduo/base/Logging.h>
using json = nlohmann::json;



ChatServer::ChatServer(EventLoop* loop,
                        const InetAddress& listenAddr,
                        const string& nameArg)
    : _server(loop, listenAddr, nameArg), _evLoop(loop){
        //注册链接的回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, placeholders::_1));
        //注册消息的回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));
        //设置线程数量
        _server.setThreadNum(4);  //一个主reactor负责连接，剩下的三个reactor为处理读写操作

        //添加定时器，每隔60s检测一次非活跃连接
        _evLoop->runEvery(60.0, bind(&ChatService::checkIdleConn, ChatService::instance()));

        //初始化心跳服务 使用TCP端口+1作为udp协议端口
        uint16_t tcpPort = listenAddr.port();  //nginx配置是6000或6002
        uint16_t udpPort = tcpPort + 1;   //变为6001 或 6003
        InetAddress udpAddr(listenAddr.toIp(), udpPort);
        LOG_INFO << "服务器启动配置 - TCP端口: " << tcpPort << ", UDP心跳端口: " << udpPort;
        ChatService::instance()->initHeartBeat(loop, udpAddr);
    }

void ChatServer::start(){
    _server.start();
}



//上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn){
    //先处理连接状态的更新
    ChatService::instance()->onConn(conn);
    //客户端断开连接
    if(!conn->connected()){
        //添加处理用户异常退出的处理方法
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}


//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time){
    string buf = buffer->retrieveAllAsString();
    //数据的反序列化
    json js = json::parse(buf);

    //更新连接的活跃时间
    ChatService::instance()->updateConnTime(conn);

    //目的是：完全解耦网络模块的代码和业务模块的代码
    //通过js["msgid"]获取一个对应的业务处理器handler
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //执行回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
}

