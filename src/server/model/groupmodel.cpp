#include "groupmodel.hpp"
#include "db.h"
#include "dbPool.hpp"
#include <iostream>



bool GroupModel::createGroup(Group &group){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());

    //从连接池中获取连接
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    if(mysql->update(sql)){
        //获取一下插入成功的用户数据生成的主键id
        group.setId(mysql_insert_id(mysql->getConnection()));
        return true;
    }
    std::cerr << "SQL执行失败: " << mysql_error(mysql->getConnection()) << std::endl;

    return false;
}



void GroupModel::addGroup(int userid, int groupid, string role){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')",
            groupid, userid, role.c_str());

    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    if(mysql->update(sql)){return;}
}



vector<Group> GroupModel::queryGroups(int userid){
    /*
    1. 先根据userid在groupuser表中查询出该用户所属的群组信息
    2. 再根据群组消息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
    */

    //组装sql语句
    char sql[1024] = {0};
    //通过 INNER JOIN 连接 AllGroup 表
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from AllGroup a inner join GroupUser b on b.groupid = a.id where b.userid = %d", userid);  

    vector<Group> groupVec;
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();
    MYSQL_RES* res = mysql->query(sql);  //调用查询操作
    if(res != nullptr){
        //把userid用户的所属群信息放入groupVec中返回
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr){
            Group group;
            group.setId(atoi(row[0]));
            group.setName(row[1]);
            group.setDesc(row[2]); 
            groupVec.push_back(group);
        }
        mysql_free_result(res); //记得释放资源
    }


    //查询群组的用户信息
    for(Group &group : groupVec){
        //根据上一步骤得到的组信息，由组的信息去进一步查询用户的详细信息
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from User a inner join GroupUser b on b.userid = a.id where b.groupid = %d", group.getID());

        MYSQL_RES* res = mysql->query(sql);  //调用查询操作
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res); //记得释放资源
        }
    }

    return groupVec;

}



vector<int> GroupModel::queryGroupUsers(int userid, int groupid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d",groupid, userid);  

    vector<int> idVec;
    shared_ptr<MySQL> mysql = dbPool::getdbPool()->getConn();    
    MYSQL_RES* res = mysql->query(sql);
    if(res != nullptr){
        //把userid用户的所属群信息放入groupVec中返回
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr){
            idVec.push_back(atoi(row[0]));
        }
        mysql_free_result(res); //记得释放资源
    }
    return idVec;
}

