#pragma once

#include "tcp_basic.hpp"

class TcpClient {
public:
    TcpClient() = default;

    bool connect(const SocketAddress& server, int timeoutMs = 3000);
    void send(const std::string& msg);
    void close();

    void setMessageCallback(TcpConnection::MessageCallback cb);
    void setCloseCallback(TcpConnection::CloseCallback cb);

private:
    TcpConnection::Ptr conn_;
};
