#include "chatserver.hpp"
#include <chatservice.hpp>
#include <iostream>
#include <signal.h>
using namespace std;

//处理服务器ctrl+c结束后，重置一下user的状态信息
void resetHandler(int){
    ChatService::instance()->reset();
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

    return 0;
}
