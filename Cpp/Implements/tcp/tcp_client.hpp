#pragma once

#include "tcp_basic.hpp"

class TcpClient {
public:
    TcpClient() = default;
    ~TcpClient();                                   // 保证读线程被 join

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
    TcpClient(TcpClient&&) noexcept = default;
    TcpClient& operator=(TcpClient&&) noexcept = default;

    // 连接服务器；timeoutMs 内未完成视为失败。
    // 已连接且未断开时直接返回 true；如需重连请先 close()。
    bool connect(const SocketAddress& server, int timeoutMs = 3000);

    // 发送一条"消息"：内部用 encodeMessage 添加 4 字节长度前缀，
    // 对端需用 decodeMessage 解帧（与 TcpConnection::MessageCallback 对称）。
    void send(const std::string& msg);

    // 发送原始字节，不加长度前缀（对端自己负责解包）。
    void sendRaw(const std::string& data);

    // 关闭连接（幂等）；会 join 读线程。
    void close();

    // 未 connect 或读线程已退出时为 false。
    bool connected() const;

    // 可先于 connect() 调用，内部缓存，connect 成功后自动生效。
    void setMessageCallback(TcpConnection::MessageCallback cb);
    void setCloseCallback(TcpConnection::CloseCallback cb);

private:
    TcpConnection::Ptr              conn_;
    TcpConnection::MessageCallback  messageCb_;
    TcpConnection::CloseCallback    closeCb_;
};