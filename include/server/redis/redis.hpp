#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include<thread>
#include <functional>
using namespace std;


class Redis{
public:
    Redis();
    ~Redis();

    //连接redis
    bool connect();
    //向redis指定的通道channel发布消息
    bool publish(int channel, string message);
    //向redis指定的通道subscribe订阅消息
    bool subscribe(int channel);
    //向redis指定通道取消订阅消息
    bool unsubscribe(int channel);
    //在独立线程中接收订阅通道中的消息
    void observser_channel_message();

    //初始化向业务层上报通道消息的回调函数
    void init_notify_handler(function<void(int, string)> fn);

private:
    //hiredis 同步上下文对象，负责publish信息
    redisContext *_publish_context;

    //hiredis 同步上下文对象，负责subscribe信息
    redisContext *_subscribe_context;

    //回调操作，收到订阅的消息给service层上报
    function<void(int, string)> _notify_message_handler;



};



#endif