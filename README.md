# chatServer
基于muduo网络库（底层基于Reactor反应堆模型，epoll和线程池、数据库连接池），运用了nginx tcp负载均衡与基于redis订阅-分发功能的集群聊天服务器和客户端源码

## 运行需求
1. nginx负载均衡配置
- cd /usr/local/nginx 再ls， 进行cd conf， 接着vim nginx.conf
- 修改配置文件，增加一段内容如下：
```shell
# 修改Nginx配置，确保UDP和TCP的会话一致性：
stream {
    # TCP配置
    upstream MyServer {
        server 127.0.0.1:6000 weight=1;
        server 127.0.0.1:6002 weight=1;
        hash $remote_addr consistent;  # 基于客户端IP的一致性哈希
    }
    
    # UDP配置
    upstream heartbeat_server {
        server 127.0.0.1:6001;
        server 127.0.0.1:6003;
        hash $remote_addr consistent;  # 使用相同的哈希策略
    }
    
    # TCP服务
    server {
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
    
    # UDP服务
    server {
        listen 8001 udp;
        proxy_pass heartbeat_server;
        proxy_timeout 1s;
    }
}
```

2. 启动redis服务，通过ps -ef | grep redis查看是否成功启动redis服务

3. 启动mysql...

## 项目编译
1. 在项目根目录中直接执行：

```shell
./autobuild
```

2. 终端执行如下命令
```
cd build
rm -rf *
cmake ..
make
```


## 项目内容
1. 使用muduo网络库作为项目的网络核心模块，提供高并发网络IO服务，解藕了网络与业务模块的代码
2. 实现了双重长连接保活机制，提高了系统稳定性和资源利用率
   - 基于UDP的心跳检测：客户端定期发送心跳包，服务端通过心跳计数器监控连接状态，及时清理异常断开的客户端
   - 空闲连接管理：服务器每30分钟检测非活跃连接，自动清理超时连接，避免资源浪费
   - 通过Nginx配置实现TCP和UDP流量的一致性路由，确保分布式环境下心跳与业务连接的正确对应
3. 使用mysql关系型数据库作为项目数据库的落地存储
   - 通过数据库连接池提高访问数据库数据的效率
4. 使用了json序列化和反序列化消息作为私有通信协议
5. 配置nginx基于tcp的负载均衡，实现聊天服务器的集群功能，提高了后端服务的并发能力
6. 基于redis的发布-订阅功能，实现跨服务器的消息通信


