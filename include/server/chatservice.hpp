#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <muduo/net/TcpConnection.h>
#include "json.hpp"
#include "userModel.hpp"
#include "offlineMessageModel.hpp"
#include "friendModel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

using json = nlohmann::json;
//表示处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

//聊天服务器业务类
class ChatService{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登陆业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //处理注销业务
    void loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理客户端异常退出的方法
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常 业务重置的方法
    void reset();

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int , string );

    
private:
    //使用单例模式,将构造函数私有化
    ChatService();
    //存储消息id和对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    //存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    //定义互斥锁保证_userConnMap线程安全
    mutex _connMutex;


    //数据操作类的对象
    UserModel _userModel;
    offlineMsgModel _offlineMsgModel;
    friendModel _friendModel;
    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;

};



#endif