#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>
#include <chrono>
using namespace std;



// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    // 给外部提供一个访问_conn的接口
    MYSQL* getConnection();


    //刷新起始的空闲时间
    void refreshAliveTime();
    //计算连接存活的总时长
    long long getAliveTime();

private:
    MYSQL *
        _conn;

    chrono::steady_clock::time_point m_aliveTime;
};

#endif