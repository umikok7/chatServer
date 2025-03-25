#ifndef DBPOOL_H
#define DBPOOL_H

#include "db.h"
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <atomic>
using namespace std;

//数据库连接池
class dbPool{
public:
    static dbPool* getdbPool();   //采取单例模式
    dbPool(const dbPool& obj) = delete;
    dbPool& operator=(const dbPool& obj) = delete;
    shared_ptr<MySQL> getConn();   //消费者
    ~dbPool();
    //添加关闭方法
    void shutdown();
    
private:
    dbPool();
    void produceConn();   //生产者
    void recycleConn();   //回收者
    void addConn();    

    string m_ip;
    string m_user;
    string m_password;
    string m_dbName;
    unsigned short m_port;
    //允许的最大、最小连接数
    int m_maxSize = 1024;
    int m_minSize = 100;
    //允许的最长等待时间和最长空闲时间
    int m_timeout = 1000;
    int m_maxIdelTime = 5000;
    queue<MySQL*> m_connQ;
    mutex m_mutex;
    condition_variable m_cond;  //用于阻塞消费者线程

    //添加一个退出标志
    atomic<bool> m_shutdown{false};

};



#endif