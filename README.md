# chatServer
基于epool和线程池、数据库连接池（待补充）可以工作在nginx tcp负载均衡环境中的集群聊天服务器和客户端源码 

## 运行需求
1. nginx负载均衡配置...
2. 启动redis服务... 
3. 启动mysql...

## 项目编译
```
cd build
rm -rf *
cmake ..
make
```
