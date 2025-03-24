#include "userModel.hpp"
#include "dbPool.hpp"
#include "db.h"
#include <iostream>

//User表的增加方法
bool UserModel::insert(User &user){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    //从连接池中获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    if(mysql->update(sql)){
        //获取一下插入成功的用户数据生成的主键id
        user.setId(mysql_insert_id(mysql->getConnection()));
        return true;
    }
    std::cerr << "SQL执行失败: " << mysql_error(mysql->getConnection()) << std::endl;

    return false;
}


User UserModel::query(int id){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d", id);
    //从连接池获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    MYSQL_RES* res = mysql->query(sql);
    if(res != nullptr){
        MYSQL_ROW row = mysql_fetch_row(res);
        if(row != nullptr){
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setPwd(row[2]);
            user.setState(row[3]);
            mysql_free_result(res);
            return user;
        }
    }

    return User();
}


bool UserModel::updateState(User user){
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    //从连接池获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    if(mysql->update(sql)){
        return true;
    }

    return false;
}


void UserModel::resetState(){
    char sql[1024] = {"update User set state = 'offline' where state = 'online' "};
    //从连接池获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    mysql->update(sql);
}


