#include "redis.hpp"
#include <iostream>

/*
同步通道发布与订阅，也可以魔改成异步的版本
*/

Redis::Redis() : _publish_context(nullptr), _subscribe_context(nullptr){

}



Redis::~Redis(){
    if(_publish_context != nullptr){
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr){
        redisFree(_subscribe_context);
    }
}



bool Redis::connect(){
    //负责publish发布信息的上下文
    _publish_context = redisConnect("127.0.0.1", 6379);
    if(nullptr == _publish_context){
        cerr << "connect redis failed!" << endl;
        return false;
    }
    // 负责subscribe订阅消息的上下文
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if(nullptr == _subscribe_context){
        cerr << "connect redis failed!" << endl;
        return false;
    }

    //在单独的线程中监听通道上的事件，有消息了则向业务层上报
    thread t([&](){
        observser_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;
    return true;

}


//向redis指定的通道subscribe订阅消息
bool Redis::publish(int channel, string message){
    //由于PUBLISH命令不会发生阻塞，所以直接调用redisCommand没有关系
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if(reply == nullptr){
        cerr << "publish command falied!" << endl;
    }
    freeReplyObject(reply);
    return true;
}


/*
1. SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，此处只做订阅通道，不接收通道消息
2. 通道消息的接收专门在observser_channel_message函数中独立线程进行
3. 只负责发送命令，不阻塞接收redis server响应命令，否则和notifyMsg线程抢占响应的资源
*/
bool Redis::subscribe(int channel){
    //由于SUBSCRIBE命令会造成阻塞，所以措施是用redisAppendCommand将数据写到缓冲区
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel)){
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    //redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done)){
            cerr << "subscribe command falied!" << endl;
            return false;
        }
    }
    // redisGetReply

    return true;
}


bool Redis::unsubscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel)){
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    //redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done)){
            cerr << "unsubscribe command falied!" << endl;
            return false;
        }
    }
    return true;
}


//在独立线程中接收订阅通道消息
void Redis::observser_channel_message(){
    redisReply *reply = nullptr;
    //以阻塞的方式读取_subscribe_context上下文中的消息
    while(REDIS_OK == redisGetReply(this->_subscribe_context, (void **)&reply)){
        //订阅收到的消息是一个带三元素的数组
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr){
            //当有消息到来的时候给业务层上报 通道 上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int, string)> fn){
    this->_notify_message_handler = fn;
}


