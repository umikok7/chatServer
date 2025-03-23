#include "friendModel.hpp"
#include "db.h"
#include <iostream>

void friendModel::insert(int userid, int friendid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d, %d)", userid, friendid);

    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){return;}
        std::cerr << "SQL执行失败: " << mysql_error(mysql.getConnection()) << std::endl;
    }
    else{std::cerr << "数据库连接失败" << std::endl;}
}



vector<User> friendModel::query(int userid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d", userid);  //通过 INNER JOIN 连接 User 表

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);  //调用查询操作
        if(res != nullptr){
            //把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]); 
                vec.push_back(user);
            }
            mysql_free_result(res); //记得释放资源
            return vec;
        }
    }
    return vec;
}