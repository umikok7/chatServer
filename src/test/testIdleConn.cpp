#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <cstring>
#include <condition_variable>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

using namespace std;

// 用于安全退出的全局变量
atomic_bool running(true);

// 模拟客户端连接
class TcpClient {
public:
    TcpClient(const string& ip, int port) 
        : _sockfd(-1), _ip(ip), _port(port), _connected(false) {}
    
    ~TcpClient() {
        disconnect();
    }
    
    // 连接服务器
    bool connect() {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0) {
            cerr << "创建socket失败" << endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(_port);
        server_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        
        if (::connect(_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "连接服务器失败: " << _ip << ":" << _port << endl;
            close(_sockfd);
            _sockfd = -1;
            return false;
        }
        
        cout << "连接到服务器: " << _ip << ":" << _port << endl;
        _connected = true;
        return true;
    }
    
    // 发送心跳消息以保持连接活跃
    bool sendHeartbeat() {
        if (!_connected) return false;
        
        // 模拟一个简单的json格式消息
        string msg = "{\"msgid\":1,\"id\":0,\"password\":\"keepalive\"}\n";
        
        // 使用MSG_NOSIGNAL防止SIGPIPE信号
        ssize_t len = send(_sockfd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
        
        if (len <= 0 || len != static_cast<ssize_t>(msg.length())) {
            cerr << "发送心跳消息失败，连接可能已断开" << endl;
            _connected = false;  // 更新连接状态
            return false;
        }
        
        // 增加连接状态检测：读取可能的响应，检测连接是否真的活跃
        char buffer[10];
        struct pollfd pfd;
        pfd.fd = _sockfd;
        pfd.events = POLLHUP | POLLERR;
        
        if (poll(&pfd, 1, 0) > 0) {
            if (pfd.revents & (POLLHUP | POLLERR)) {
                cerr << "连接已断开" << endl;
                _connected = false;
                return false;
            }
        }
        
        return true;
    }

    bool checkConnection() {
        if (!_connected) return false;
        
        // 使用更强制的方式检测连接状态 - 尝试写入一个字节
        char dummy = 0;
        int result = send(_sockfd, &dummy, 0, MSG_NOSIGNAL);
        if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            cout << "连接已断开(send测试失败): " << strerror(errno) << endl;
            _connected = false;
            return false;
        }
        
        // 使用poll检测连接状态
        struct pollfd pfd;
        pfd.fd = _sockfd;
        pfd.events = POLLIN | POLLHUP | POLLERR;
        
        if (poll(&pfd, 1, 0) > 0) {
            // 检查连接是否已关闭
            if (pfd.revents & (POLLHUP | POLLERR)) {
                cout << "连接已断开(POLLHUP/POLLERR)" << endl;
                _connected = false;
                return false;
            }
            
            // 如果有数据可读，尝试读取以检查是否为FIN
            if (pfd.revents & POLLIN) {
                char buffer[1];
                int n = recv(_sockfd, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
                if (n == 0) {
                    cout << "连接已断开(收到FIN)" << endl;
                    _connected = false;
                    return false;
                }
                else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    cout << "连接已断开(recv错误): " << strerror(errno) << endl;
                    _connected = false;
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // 断开连接
    void disconnect() {
        if (_sockfd != -1) {
            close(_sockfd);
            _sockfd = -1;
            _connected = false;
            cout << "客户端断开连接" << endl;
        }
    }
    
    bool isConnected() const {
        return _connected;
    }
    
private:
    int _sockfd;
    string _ip;
    int _port;
    bool _connected;
};

// 信号处理函数
void signalHandler(int signum) {
    cout << "接收到信号: " << signum << ", 准备退出..." << endl;
    running = false;
}

int main() {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    
    const string SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 8000;  // 替换为你的服务器端口
    
    cout << "测试非活跃连接检测功能..." << endl;
    
    // 创建10个客户端连接
    vector<shared_ptr<TcpClient>> clients;
    for (int i = 0; i < 10; ++i) {
        auto client = make_shared<TcpClient>(SERVER_IP, SERVER_PORT);
        if (client->connect()) {
            clients.push_back(client);
            
            // 初始心跳，确保所有连接都被正确建立
            client->sendHeartbeat();
            
            // 短暂等待，避免连接过快
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    
    cout << "成功创建 " << clients.size() << " 个连接" << endl;
    
    if (clients.empty()) {
        cerr << "没有成功建立连接，测试终止" << endl;
        return 1;
    }
    
    cout << "一半的连接会保持活跃发送心跳，另一半将保持空闲" << endl;
    cout << "按Ctrl+C终止测试" << endl;
    
    int count = 0;
    while (running) {
        // 每5秒检查一次连接状态
        this_thread::sleep_for(chrono::seconds(5));
        
        cout << "心跳检测周期: " << ++count << endl;
        
        // 只有一半的客户端发送心跳包
        for (size_t i = 0; i < clients.size(); ++i) {
            if (i % 2 == 0 && clients[i]->isConnected()) {
                cout << "客户端 " << i << " 发送心跳包" << endl;
                if (!clients[i]->sendHeartbeat()) {
                    cout << "客户端 " << i << " 心跳失败，可能已断开" << endl;
                }
            }
        }

        // 对所有客户端执行连接状态检测
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i]->isConnected()) {
                // 主动检测连接状态
                if (!clients[i]->checkConnection()) {
                    cout << "客户端 " << i << " 连接已断开" << endl;
                }
            }
        }
        
        // 检查连接状态
        size_t active = 0;
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i]->isConnected()) {
                active++;
            }
        }
        cout << "当前活跃连接数: " << active << "/" << clients.size() << endl;
        
        // 如果运行超过 分钟，应该看到非活跃连接被关闭
        if (count * 5 > 2 * 60) {
            cout << "测试时间已超过2分钟，检查非活跃连接是否已关闭" << endl;
            
            size_t expected_active = 0;
            for (size_t i = 0; i < clients.size(); ++i) {
                if (i % 2 == 0) {  // 应该只有偶数索引的客户端还保持连接
                    expected_active++;
                }
            }
            
            cout << "预期活跃连接: " << expected_active << ", 实际活跃连接: " << active << endl;
            if (active <= expected_active) {
                cout << "测试成功: 非活跃连接已被服务器关闭" << endl;
                running = false;  // 测试成功后自动退出
            }
        }
    }
    
    cout << "清理连接并退出..." << endl;
    clients.clear();
    
    return 0;
}
