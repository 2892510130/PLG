#pragma once

#include <mutex>
#include <unordered_map>
#include <functional>
#include <atomic>

#include "tcp_basic.hpp"

class TcpServer {
public:
    using ConnectionCallback = std::function<void(const TcpConnection::Ptr&)>;

    explicit TcpServer(const SocketAddress& listenAddr);

    void setConnectionCallback(ConnectionCallback cb);
    void setMessageCallback(TcpConnection::MessageCallback cb);

    void start();   // 阻塞在 accept 循环；内部 spawn 读线程
    void stop();

private:
    void acceptLoop();
    void removeConnection(int fd);

private:
    Socket listenSocket_;
    std::unordered_map<int, TcpConnection::Ptr> conns_;  // fd → 连接
    std::mutex connsMutex_;
    ConnectionCallback onConnection_;
    TcpConnection::MessageCallback onMessage_;
    std::atomic<bool> running_{false};
};
