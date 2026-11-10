#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "tcp_basic.hpp"

class TcpServer {
public:
    using ConnectionCallback = std::function<void(const TcpConnection::Ptr&)>;
    using CloseCallback      = TcpConnection::CloseCallback;

    explicit TcpServer(const SocketAddress& listenAddr);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    void setConnectionCallback(ConnectionCallback cb);
    void setMessageCallback(TcpConnection::MessageCallback cb);
    void setCloseCallback(CloseCallback cb);               // 新增

    // 阻塞在 accept 循环，直到 stop() 从其他线程被调用。
    // 典型用法：
    //   std::thread t([&]{ server.start(); });
    //   ...
    //   server.stop();
    //   t.join();
    void start();

    // 唤醒 accept、关闭所有连接（会 join 各自的读线程）。
    // 可从任意线程调用，但**不要**从 TcpServer 自己的连接回调线程里调用 stop，
    // 否则等于 join 自己。若需从回调中停止服务，请在回调里 notify 一个
    // 外部线程去调用 stop()。
    void stop();

private:
    void acceptLoop();
    void removeConnection(int fd);

private:
    Socket                                        listenSocket_;
    std::unordered_map<int, TcpConnection::Ptr>   conns_;      // fd → 连接
    std::mutex                                    connsMutex_;

    ConnectionCallback                            onConnection_;
    TcpConnection::MessageCallback                onMessage_;
    CloseCallback                                 onClose_;
    std::mutex                                    cbMutex_;

    std::atomic<bool>                             running_{false};
};