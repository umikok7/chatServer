#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h> 
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;

/*基于moduo网络库开发服务器程序 优点:将网络的I/O代码和业务代码分开,也就是将精力都放在onConnection()和onMessage()上
1. 组合TcpServer对象
2. 创建EventLoop事件循环对象指针
3. 明确TcpServer创建函数需要什么参数，输出chatserver的构造函数
4. 在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5. 设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/

class ChatServer{
public:
    ChatServer(EventLoop* loop,  //事件循环
        const InetAddress& listenAddr,  // IP+Port
        const string& nameArg) //服务器名称
         : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        //给服务器注册用户连接的创建和断开回调
        //注意：onConnection是成员函数，隐含了一个this指针，为了补上这个隐含的this指针需要使用可调用对象绑定器绑定一个this,使用这个this指针来调用正确对象的成员函数。
        auto it = std::bind(&ChatServer::onConnection, this, placeholders::_1);
        _server.setConnectionCallback(it);

        //给服务器注册用户读写事件的回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

        //设置服务器端的线程数量   
        _server.setThreadNum(4);  //此处一个I/O线程，三个worker线程

    }

    //开启事件循环
    void start(){
        _server.start();
    }

private:
    //专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr &conn){
        if(conn->connected()){
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state: online" << endl;
        }
        else{
            cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state: offline" << endl;
            conn->shutdown();  //也就是close(fd)
            //_loop->quit();
        }
    }

    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, Buffer* buffer, Timestamp time){
        string buf = buffer->retrieveAllAsString();
        cout << "recv data is: " << buf << " time is: " << time.toString() << endl;
        conn->send(buf);
    }

                    

    TcpServer _server;
    EventLoop *_loop;
};

int main(){
    EventLoop evLoop;  //epool
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&evLoop, addr, "ChatServer");

    server.start();  //让listenfd epoll_ctl上树
    evLoop.loop();  //epoll_wait 以阻塞方式等待新用户的连接，已连接用户的读写事件等

    return 0;

}