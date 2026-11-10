#include "tcp_client.hpp"

#include <poll.h>
#include <cerrno>
#include <stdexcept>

// ============================================================ 生命周期
TcpClient::~TcpClient() {
    close();
}

// ============================================================ connect
bool TcpClient::connect(const SocketAddress& server, int timeoutMs) {
    // 已连接且未断开
    if (conn_ && !conn_->closed()) return true;
    // 已断开：丢掉旧连接，走重连流程
    conn_.reset();

    try {
        Socket sock;                        // socket(AF_INET, SOCK_STREAM, 0)
        sock.setNonBlocking(true);          // 必须先非阻塞，poll 才有意义

        int rc = ::connect(sock.fd(), server.data(), server.length());
        if (rc < 0) {
            if (errno != EINPROGRESS) return false;

            // 等待可写 or 出错
            pollfd pfd{};
            pfd.fd     = sock.fd();
            pfd.events = POLLOUT;

            int n;
            do {
                n = ::poll(&pfd, 1, timeoutMs);
            } while (n < 0 && errno == EINTR);

            if (n <= 0) return false;       // 0 = 超时，<0 = 出错

            // 取连接结果：成功时 SO_ERROR == 0
            int err = 0;
            socklen_t elen = sizeof(err);
            if (::getsockopt(sock.fd(), SOL_SOCKET, SO_ERROR, &err, &elen) < 0)
                return false;
            if (err != 0) return false;
        }

        // readLoop 是阻塞 recv，必须切回阻塞模式
        sock.setNonBlocking(false);
        sock.setTcpNoDelay(true);

        conn_ = std::make_shared<TcpConnection>(std::move(sock), server);
        if (messageCb_) conn_->setMessageCallback(messageCb_);
        if (closeCb_)   conn_->setCloseCallback(closeCb_);
        conn_->start();
        return true;
    } catch (const std::exception&) {
        conn_.reset();                      // 保险：异常时不留残状态
        return false;
    }
}

// ============================================================ send
void TcpClient::send(const std::string& msg) {
    if (!conn_) throw std::runtime_error("TcpClient::send: not connected");
    conn_->send(encodeMessage(msg));        // 加长度前缀，与 decodeMessage 对称
}

void TcpClient::sendRaw(const std::string& data) {
    if (!conn_) throw std::runtime_error("TcpClient::sendRaw: not connected");
    conn_->send(data);
}

// ============================================================ close
void TcpClient::close() {
    if (conn_) {
        conn_->close();     // 内部 shutdown 唤醒读线程，再 join
        conn_.reset();
    }
}

bool TcpClient::connected() const {
    return conn_ && !conn_->closed();
}

// ============================================================ callbacks
void TcpClient::setMessageCallback(TcpConnection::MessageCallback cb) {
    messageCb_ = std::move(cb);
    if (conn_) conn_->setMessageCallback(messageCb_);
}

void TcpClient::setCloseCallback(TcpConnection::CloseCallback cb) {
    closeCb_ = std::move(cb);
    if (conn_) conn_->setCloseCallback(closeCb_);
}