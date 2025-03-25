#include "dbPool.hpp"
#include <thread>



dbPool* dbPool::getdbPool(){
    static dbPool pool;
    return &pool;
}


void dbPool::addConn(){
    MySQL* conn = new MySQL;
    conn->connect();
    m_connQ.push(conn);
}


dbPool::dbPool(){
    //初始化连接池
    for(int i = 0; i < m_minSize; i++){
        addConn();
    }
    thread producer(&dbPool::produceConn, this);
    thread recycler(&dbPool::recycleConn, this);
    producer.detach();
    recycler.detach();
}


//生产者
void dbPool::produceConn(){
    while (!m_shutdown){
        unique_lock<mutex> lock(m_mutex);
        while(m_connQ.size() >= m_minSize && !m_shutdown){
            // 此时连接数充足，等待消费者使用
            m_cond.wait(lock);
        }
        if(m_shutdown) break;  // 如果被唤醒是因为需要退出，则跳出循环
        addConn();
        m_cond.notify_all();
    }
    
}


//回收者
void dbPool::recycleConn(){
    while(!m_shutdown){
        this_thread::sleep_for(chrono::milliseconds(500));
        if(m_shutdown) break;   // 如果被唤醒是因为需要退出，则跳出循环

        lock_guard<mutex> lock(m_mutex);
        while(m_connQ.size() > m_minSize){
            //只有数量大于最小的数量的情况下才进行回收
            MySQL* conn = m_connQ.front();
            //只有大于最大空闲时间的时候才能进行回收
            if(conn->getAliveTime() >= m_maxIdelTime) {
                m_connQ.pop();
                delete conn;
            }
            else{
                break;
            }
        }
    }
}



//消费者
shared_ptr<MySQL> dbPool::getConn(){
    unique_lock<mutex> lock(m_mutex);
    //条件变量超时等待机制，不会无限的等待，定期唤醒线程
    while(m_connQ.empty()){
        if(cv_status::timeout == m_cond.wait_for(lock, chrono::milliseconds(m_timeout))){
            if(m_connQ.empty()){continue;} //再次进行判断能够避免虚假唤醒
        }
    }
    //只有在任务队列不为空的时候才进行池的连接
    //此处自定义了共享的智能指针的析构方式
    shared_ptr<MySQL> connPtr(m_connQ.front(), [this](MySQL* conn){
        //此处要对m_connQ进行写操作，共享资源记得加锁
        lock_guard<mutex> lock(m_mutex);
        conn->refreshAliveTime();
        m_connQ.push(conn);      //使用后销毁的时候放回连接池当中
    });
    m_connQ.pop();
    m_cond.notify_all();

    return connPtr;
}



dbPool::~dbPool(){
    while(!m_connQ.empty()){
        MySQL* conn = m_connQ.front();
        m_connQ.pop();
        delete conn;
    }
}

void dbPool::shutdown(){
    m_shutdown = true;
    m_cond.notify_all(); //唤醒所有等待的线程
}

