#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <mutex>
#include <atomic>
#include <thread>

class SocketAddress {
public:
    static SocketAddress any(uint16_t port);                    // 0.0.0.0:port
    static SocketAddress parse(const std::string& ip, uint16_t port);

    const sockaddr* data() const;
    socklen_t length() const;
    std::string ip() const;
    uint16_t port() const;
private:
    sockaddr_storage addr_{};
    socklen_t len_ = 0;
};

class Socket {
public:
    Socket();                          // socket(AF_INET, SOCK_STREAM, 0)
    explicit Socket(int fd);           // 接管已有 fd
    ~Socket();                         // close(fd_)
    Socket(Socket&&) noexcept;         // 移动语义
    Socket& operator=(Socket&&) noexcept;
    Socket(const Socket&) = delete;

    int  fd() const;
    bool valid() const;
    void close();

    // client
    void bind(const SocketAddress&);
    void listen(int backlog = SOMAXCONN);
    Socket accept(SocketAddress* peer = nullptr);   // 返回新 Socket

    // server
    void connect(const SocketAddress&);
    
    // send and receive
    size_t send(const void* buf, size_t len, int flags = 0);
    size_t recv(void* buf, size_t len, int flags = 0);
    void    sendAll(const void* buf, size_t len);   // 循环直到发完
    void    shutdownWrite();

    // settings
    void setReuseAddr(bool on);
    void setNonBlocking(bool on);
    void setTcpNoDelay(bool on);
    void setKeepAlive(bool on);
private:
    int fd_ = -1;
};

class Buffer {
public:
    void   append(const void* data, size_t len);
    void   append(const std::string& s);
    size_t readableBytes() const;
    const char* peek() const;              // 指向可读数据起始
    void   retrieve(size_t len);           // 消费 len 字节
    std::string retrieveAllAsString();
    void   clear();
private:
    std::vector<char> data_;
    size_t readIdx_ = 0;
};

// 长度前缀协议：4 字节网络序长度 + payload
std::string encodeMessage(const std::string& payload);

// 从 Buffer 中尝试解出一帧；成功返回 true 并填充 msg
bool decodeMessage(Buffer& buf, std::string& msg);

struct SocketError : std::runtime_error { int err; };

// 或 return-based:
enum class IoResult { Ok, WouldBlock, Closed, Error };

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using Ptr = std::shared_ptr<TcpConnection>;
    using MessageCallback = std::function<void(const Ptr&, Buffer*)>;
    using CloseCallback   = std::function<void(const Ptr&)>;

    TcpConnection(Socket sock, SocketAddress peer);

    void start();                               // 启动读线程（或注册事件）
    void send(const void* data, size_t len);    // 线程安全：加锁 + sendAll
    void send(const std::string& s);
    void close();

    void setMessageCallback(MessageCallback cb);
    void setCloseCallback(CloseCallback cb);
    const SocketAddress& peer() const;

private:
    void readLoop();          // 循环 recv → append 到 inputBuffer_ → 解帧 → 回调
private:
    Socket socket_;
    SocketAddress peer_;
    Buffer inputBuffer_;
    MessageCallback onMessage_;
    CloseCallback   onClose_;
    std::mutex sendMutex_;
    std::thread readThread_; // 简化版：每连接一线程
    std::atomic<bool> closed_{false};
};
