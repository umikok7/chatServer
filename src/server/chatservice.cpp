#include "chatservice.hpp"
#include "public.hpp"
#include "chatserver.hpp"
#include <vector>
#include <muduo/base/Logging.h>
using namespace muduo;


//获取单例对象的接口函数
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

//注册消息和对应的Handler回调操作
ChatService::ChatService(){
    //用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({EnMsgType::LOGIN_MSG, std::bind(&ChatService::login, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({EnMsgType::LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({EnMsgType::REG_MSG, std::bind(&ChatService::reg, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({EnMsgType::ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({EnMsgType::ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, placeholders::_1, placeholders::_2, placeholders::_3)});

    //群组业务的相关回调
    _msgHandlerMap.insert({EnMsgType::CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({EnMsgType::ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({EnMsgType::GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, placeholders::_1, placeholders::_2, placeholders::_3)});

    //连接redis服务器
    if(_redis.connect()){
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, placeholders::_1, placeholders::_2));
    }
}


void ChatService::reset(){
    //将online状态的用户设置为offline
    _userModel.resetState();
}


MsgHandler ChatService::getHandler(int msgid){
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        //返回一个默认的处理器()中放入function的参数
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else{
        return _msgHandlerMap[msgid];
    }
}


//处理登陆业务 id, pwd    ORM框架：业务层处理的都是对象，全无sql语句
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id && user.getPwd() == pwd){
        if(user.getState() == "online"){
            //用户已经登陆，不可重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登陆";
            conn->send(response.dump());
        }
        else{
            {
                //登陆成功 记录一下用户的连接信息   
                lock_guard<mutex> lock(_connMutex);  //一定要加锁，因为onMessage反复的调用回调函数时，此处的_userConnMap可能会被多线程访问，加锁保障线程安全
                _userConnMap.insert({id, conn});    
            }

            //id用户登陆成功后，向redis订阅channel（id）
            _redis.subscribe(id);

            //登陆成功，更新用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            json response;
            //响应回去的数据
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                //先获取离线消息 然后将离线消息从数据库删除
                response["offlinemsg"] = vec;
                _offlineMsgModel.remove(id);
            }
            //查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User &user : userVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.emplace_back(js.dump());
                }
                response["friends"] = vec2;   //就是你个b？
            }
            //查询该用户的群消息并返回
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if(!groupuserVec.empty()){
                vector<string> groupV;
                for(Group &group : groupuserVec){
                    json groupJs;
                    groupJs["id"] = group.getID();
                    groupJs["groupname"] = group.getName();
                    groupJs["groupdesc"] = group.getDesc();

                    vector<string> userV;
                    for(GroupUser &user : group.getUsers()){
                        json userJs;
                        userJs["id"] = user.getId();
                        userJs["name"] = user.getName();
                        userJs["state"] = user.getState();
                        userJs["role"] = user.getRole();
                        userV.push_back(userJs.dump());
                    }

                    groupJs["users"] = userV;
                    groupV.push_back(groupJs.dump());
                }
                response["groups"] = groupV;
            }

            conn->send(response.dump()); //.dump()方法将字符串扁平化后发送出去
        }
    }
    else{
        //用户不存在或者用户存在密码错误 登陆失败
        json response;
        //响应回去的数据
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误";
        conn->send(response.dump()); //.dump()方法将字符串扁平化后发送出去
    }
}


void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end()){
            _userConnMap.erase(it);
        }
    }

    //用户注销，相当于下线，在redis中取消订阅
    _redis.unsubscribe(userid);

    //更新用户状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}




//注册业务 填入 name 和 password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time){
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    user.setState("offline");
    bool state = _userModel.insert(user);
    if(state){
        //注册成功
        json response;
        response["msgid"] = REG_MGS_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump()); //.dump()方法将字符串扁平化后发送出去
    }
    else{
        //注册失败
        json response;
        response["msgid"] = REG_MGS_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }

}



void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        //1. 从哈希表中删除
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); it++){
            if(it->second == conn){
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户注销，相当于下线，在redis中取消订阅
    _redis.unsubscribe(user.getId());

    //2. 将状态从online修改为offline
    if(user.getId() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}



void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int toId = js["to"].get<int>();

    {
        lock_guard<mutex> locker(_connMutex);
        auto it = _userConnMap.find(toId); 
        if(it != _userConnMap.end()){
            //在线,可以转发消息   服务器主动推送消息给toId用户,也就是通过服务器中转将消息发送给对端 因为"to"字段对应的conn调用send，服务器会把对应数据发送给这个调用send的客户端
            it->second->send(js.dump());
            return;
        }
    }

    //查询toid是否在线
    User user = _userModel.query(toId);
    if(user.getState() == "online"){
        //说明在线只不过不在当前服务器登陆的，所以_userConnMap中查询不到
        _redis.publish(toId, js.dump());
        return;
    }

    //toId对应的不在线,存储离线消息
    _offlineMsgModel.insert(toId, js.dump());
    //先存储好，在登陆成功的时候响应

}



// msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid, friendid);

    //先存储好，在登陆成功的时候响应

}



void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组消息
    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        //存储群组创建人的信息
        _groupModel.addGroup(userid, group.getID(), "creator");
    }
}



void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}



void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            //转发群消息
            it->second->send(js.dump());
        }
        else{
            //先看toid是否在线
            User user = _userModel.query(id);
            if(user.getState() == "online"){
                _redis.publish(id, js.dump());
            }
            else{
            //离线群消息
            //_offlineMsgModel.insert(userid, js.dump());
            //上述代码错误的将离线消息发送给了消息的发送者，应该要发送给其他的人
            _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}


// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg){
    json js = json::parse(msg.c_str());

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()){
        it->second->send(msg);
        return;
    }

    //存储用户的离线信息
    _offlineMsgModel.insert(userid, msg);

}


void ChatService::onConn(const TcpConnectionPtr& conn){
    if(conn->connected()){
        // 连接建立时记录活跃时间
        updateConnTime(conn);
    }else{
        // 连接断开时清理连接活跃时间记录
        lock_guard<mutex> lock(_connMutex);
        _connTimeMap.erase(conn);
    }
}


// 更新连接活跃时间
void ChatService::updateConnTime(const TcpConnectionPtr& conn){
    lock_guard<mutex> lock(_connMutex);
    _connTimeMap[conn] = Timestamp::now();
}


// 检查非活动连接
void ChatService::checkIdleConn(){
    lock_guard<mutex> lock(_connMutex);
    Timestamp now = Timestamp::now();
    vector<pair<TcpConnectionPtr, int>> connToClose;  // 存储<连接,用户ID>对

    //先找出所有需要关闭的连接
    for(auto it = _connTimeMap.begin(); it != _connTimeMap.end(); ){
        //计算连接空闲时间
        double idleTime = timeDifference(now, it->second);
        // 连接超时，执行断开连接的处理
        if(idleTime > idleSeconds){
            LOG_INFO << "检测到空闲连接： " << it->first->peerAddress().toIpPort()
                    << " 空闲时间： " << idleTime << "s";
            
            int userid = -1;
            //查找连接对应用户id
            for(const auto& userConn : _userConnMap){
                if(userConn.second == it->first){
                    userid = userConn.first;
                    break;
                }
            }    
            //将连接保存到待关闭列表
            connToClose.push_back({it->first, userid});
            // 删除活跃时间记录
            it = _connTimeMap.erase(it);  //正确处理迭代器，不记录的话就没法循环继续了
        }else{
            ++it;
        }    
    }        
    // 处理需要关闭的连接
    for(const auto& item : connToClose){
        const TcpConnectionPtr& conn = item.first;
        int userId = item.second;      
        if(userId != -1){
            // 处理已登录用户的连接关闭
            // 取消redis订阅
            _redis.unsubscribe(userId);
            // 设置用户为离线状态
            User user(userId, "", "", "offline");
            _userModel.updateState(user);
            // 从用户连接映射表中删除(使用用户ID直接删除)
            _userConnMap.erase(userId);
        }
        
        // 最后再关闭连接
        LOG_INFO << "关闭空闲连接： " << conn->peerAddress().toIpPort();
        
        // 使用shared_from_this确保在操作期间连接不会被销毁
        auto guardedConn = conn->shared_from_this();
        guardedConn->getLoop()->runInLoop([guardedConn](){
            guardedConn->forceClose();
        });
    }
}

    




