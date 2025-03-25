# chatServer
基于muduo网络库（底层基于Reactor反应堆模型，epoll和线程池、数据库连接池），运用了nginx tcp负载均衡与基于redis订阅-分发功能的的集群聊天服务器和客户端源码

## 运行需求
1. nginx负载均衡配置
- cd /usr/local/nginx 再ls， 进行cd conf， 接着vim nginx.conf
- 修改配置文件，增加一段内容如下：
```
# nginx tcp loadbalance config
stream {
        upstream MyServer{
                server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
                server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
        }

        server{
                proxy_connect_timeout 1s;
                #proxy_timeout 3s;
                listen 8000;
                proxy_pass MyServer;
                tcp_nodelay on;
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
2. 使用了json序列化和反序列化消息作为私有通信协议
3. 使用mysql关系型数据库作为项目数据库的落地存储
4. 使用了数据库连接池提高访问数据库数据的效率
5. 配置nginx基于tcp的负载均衡，实现聊天服务器的集群功能，提高了后端服务的并发能力
6. 基于redis的发布-订阅功能，实现跨服务器的消息通信


