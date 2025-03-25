#include "offlineMessageModel.hpp"
#include "db.h"
#include "dbPool.hpp"




void offlineMsgModel::insert(int userid, string msg){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    //从连接池中获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    if(mysql->update(sql)){return;}
    std::cerr << "SQL执行失败: " << mysql_error(mysql->getConnection()) << std::endl; 

}


void offlineMsgModel::remove(int userid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

    //从连接池中获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    if(mysql->update(sql)){return;}
    std::cerr << "SQL执行失败: " << mysql_error(mysql->getConnection()) << std::endl; 
}


vector<string> offlineMsgModel::query(int userid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<string> vec;
    //从连接池获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    MYSQL_RES* res = mysql->query(sql);
    if(res != nullptr){
        //把userid用户的所有离线消息放入vec中返回
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr){
            vec.push_back(row[0]);
            mysql_free_result(res);  //记得释放资源
            return vec;
        }
    }
    //if不成立的话直接返回空的容器即可
    return vec;
}


