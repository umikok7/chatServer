#include "offlineMessageModel.hpp"
#include "db.h"

void offlineMsgModel::insert(int userid, string msg){
        //组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());
    
        MySQL mysql;
        if(mysql.connect()){
            if(mysql.update(sql)){return;}
            std::cerr << "SQL执行失败: " << mysql_error(mysql.getConnection()) << std::endl;
        }
        else{std::cerr << "数据库连接失败" << std::endl;}
}

void offlineMsgModel::remove(int userid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid = %d", userid);

    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){return;}
        std::cerr << "SQL执行失败: " << mysql_error(mysql.getConnection()) << std::endl;
    }
    else{std::cerr << "数据库连接失败" << std::endl;}
}


vector<string> offlineMsgModel::query(int userid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<string> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);  //调用查询操作
        if(res != nullptr){
            //把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                vec.push_back(row[0]);
            }
            mysql_free_result(res); //记得释放资源
            return vec;
        }
    }
    return vec;
}