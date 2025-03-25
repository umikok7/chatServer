#include "chatserver.hpp"
#include "dbPool.hpp"
#include <chatservice.hpp>
#include <iostream>
#include <signal.h>
using namespace std;


//处理服务器ctrl+c结束后，重置一下user的状态信息
void resetHandler(int){
    ChatService::instance()->reset();
    dbPool::getdbPool()->shutdown();   //关闭一下子线程
    exit(0);
}


int main(int argc, char **argv){

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop;
    //InetAddress addr("127.0.0.1", 6000);
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    // 事件循环结束后

    // 正常退出时也清理资源
    ChatService::instance()->reset();
    dbPool::getdbPool()->shutdown();

    return 0;
}

