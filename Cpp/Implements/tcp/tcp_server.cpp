#include "tcp_server.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>

// ============================================================ 生命周期
TcpServer::TcpServer(const SocketAddress& listenAddr) {
    listenSocket_.setReuseAddr(true);
    listenSocket_.bind(listenAddr);
    listenSocket_.listen();
}

TcpServer::~TcpServer() {
    // 用户应在析构前显式调用 stop() 并 join 调用 start() 的线程。
    // 这里做最后的兜底：唤醒 accept + 关闭连接，不负责 join accept 线程
    // （那个线程是用户起的，析构时无法替你 join 它）。
    if (running_.load()) {
        try { stop(); } catch (...) {}
    }
}

// ============================================================ callbacks
void TcpServer::setConnectionCallback(ConnectionCallback cb) {
    std::lock_guard<std::mutex> lk(cbMutex_);
    onConnection_ = std::move(cb);
}

void TcpServer::setMessageCallback(TcpConnection::MessageCallback cb) {
    std::lock_guard<std::mutex> lk(cbMutex_);
    onMessage_ = std::move(cb);
}

void TcpServer::setCloseCallback(CloseCallback cb) {
    std::lock_guard<std::mutex> lk(cbMutex_);
    onClose_ = std::move(cb);
}

// ============================================================ start / stop
void TcpServer::start() {
    if (running_.exchange(true)) {
        throw std::runtime_error("TcpServer::start: already running");
    }
    try {
        acceptLoop();
    } catch (...) {
        running_.store(false);
        throw;
    }
    running_.store(false);
}

void TcpServer::stop() {
    // 幂等：只有把 running_ 从 true 翻成 false 的那一次执行清理
    if (!running_.exchange(false)) return;

    // 唤醒阻塞在 accept() 的线程
    // 用 shutdown 而非 close：避免多线程场景下 fd 被复用导致误操作
    ::shutdown(listenSocket_.fd(), SHUT_RDWR);

    // 在锁外 close 连接：close() 会 join 各自的读线程，
    // 而读线程在 onClose 回调里要拿 connsMutex_ 调 removeConnection，
    // 若我们持锁 join 就会死锁
    std::vector<TcpConnection::Ptr> toClose;
    {
        std::lock_guard<std::mutex> lk(connsMutex_);
        toClose.reserve(conns_.size());
        for (auto& kv : conns_) toClose.push_back(kv.second);
        conns_.clear();
    }
    for (auto& c : toClose) {
        try { c->close(); } catch (...) {}
    }
}

// ============================================================ 内部
void TcpServer::removeConnection(int fd) {
    std::lock_guard<std::mutex> lk(connsMutex_);
    conns_.erase(fd);
}

void TcpServer::acceptLoop() {
    while (running_.load()) {
        SocketAddress peer;
        Socket sock;
        try {
            sock = listenSocket_.accept(&peer);
        } catch (const SocketError& e) {
            if (!running_.load()) break;                       // stop() 打断
            if (e.err == EINTR || e.err == ECONNABORTED) continue;
            if (e.err == EMFILE || e.err == ENFILE) {
                // fd 耗尽：短暂退避后重试，避免 CPU 空转
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            break;                                             // 未预期错误
        }

        if (!running_.load()) {
            sock.close();
            break;
        }

        int fd = sock.fd();
        auto conn = std::make_shared<TcpConnection>(std::move(sock), peer);

        // 内部 onClose：先移除 map 条目，再转发用户回调
        conn->setCloseCallback([this, fd](const TcpConnection::Ptr& c) {
            removeConnection(fd);

            CloseCallback userCb;
            {
                std::lock_guard<std::mutex> lk(cbMutex_);
                userCb = onClose_;
            }
            if (userCb) {
                try { userCb(c); } catch (...) {}
            }
        });

        // 快照当前的回调
        TcpConnection::MessageCallback msgCb;
        ConnectionCallback             connCb;
        {
            std::lock_guard<std::mutex> lk(cbMutex_);
            msgCb  = onMessage_;
            connCb = onConnection_;
        }
        if (msgCb) conn->setMessageCallback(std::move(msgCb));

        // 加入 map 前再检查一次 running_：避免 stop 之后 accept 到的连接被误处理
        {
            std::lock_guard<std::mutex> lk(connsMutex_);
            if (!running_.load()) {
                // 服务器正在停止：丢弃该连接，不通知用户
                // conn 析构时会自动 close socket（未 start，无读线程可 join）
                continue;
            }
            conns_[fd] = conn;
        }

        if (connCb) {
            try { connCb(conn); } catch (...) {}
        }

        // start() 会 spawn 读线程；即便 connCb 里已经 close 了也能安全启动
        conn->start();
    }
}