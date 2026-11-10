#include "tcp_basic.hpp"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <utility>

// ---------------------------------------------------------------- 内部辅助
namespace {

[[noreturn]] void throwSocketError(const char* what) {
    int e = errno;
    throw SocketError(what, e);
}

constexpr uint32_t kMaxMessageSize = 64u * 1024 * 1024; // 64 MiB 防御性上限

} // namespace

// ---------------------------------------------------------------- SocketAddress
SocketAddress SocketAddress::any(uint16_t port) {
    SocketAddress sa;
    auto* sin = reinterpret_cast<sockaddr_in*>(&sa.addr_);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = htonl(INADDR_ANY);
    sin->sin_port = htons(port);
    sa.len_ = sizeof(sockaddr_in);
    return sa;
}

SocketAddress SocketAddress::parse(const std::string& ip, uint16_t port) {
    SocketAddress sa;
    auto* sin = reinterpret_cast<sockaddr_in*>(&sa.addr_);
    sin->sin_family = AF_INET;
    sin->sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &sin->sin_addr) != 1) {
        throw std::invalid_argument("invalid IPv4 address: " + ip);
    }
    sa.len_ = sizeof(sockaddr_in);
    return sa;
}

SocketAddress SocketAddress::fromSockaddr(const sockaddr* sa, socklen_t len) {
    SocketAddress out;
    if (len > static_cast<socklen_t>(sizeof(out.addr_)))
        len = static_cast<socklen_t>(sizeof(out.addr_));
    std::memcpy(&out.addr_, sa, len);
    out.len_ = len;
    return out;
}

const sockaddr* SocketAddress::data() const {
    return reinterpret_cast<const sockaddr*>(&addr_);
}

socklen_t SocketAddress::length() const { return len_; }

std::string SocketAddress::ip() const {
    char buf[INET6_ADDRSTRLEN] = {};
    switch (addr_.ss_family) {
    case AF_INET: {
        const auto* sin = reinterpret_cast<const sockaddr_in*>(&addr_);
        if (!inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf))) return {};
        return buf;
    }
    case AF_INET6: {
        const auto* sin6 = reinterpret_cast<const sockaddr_in6*>(&addr_);
        if (!inet_ntop(AF_INET6, &sin6->sin6_addr, buf, sizeof(buf))) return {};
        return buf;
    }
    default:
        return {};
    }
}

uint16_t SocketAddress::port() const {
    switch (addr_.ss_family) {
    case AF_INET:
        return ntohs(reinterpret_cast<const sockaddr_in*>(&addr_)->sin_port);
    case AF_INET6:
        return ntohs(reinterpret_cast<const sockaddr_in6*>(&addr_)->sin6_port);
    default:
        return 0;
    }
}

// ---------------------------------------------------------------- SocketError
SocketError::SocketError(const std::string& msg, int e)
    : std::runtime_error(msg + ": " + std::strerror(e)), err(e) {}

// ---------------------------------------------------------------- Socket
Socket::Socket() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) throwSocketError("socket");
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() { close(); }

Socket::Socket(Socket&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }

Socket& Socket::operator=(Socket&& o) noexcept {
    if (this != &o) {
        close();
        fd_ = o.fd_;
        o.fd_ = -1;
    }
    return *this;
}

int  Socket::fd() const    { return fd_; }
bool Socket::valid() const { return fd_ >= 0; }

void Socket::close() {
    if (fd_ >= 0) {
        ::close(fd_);   // 若 readLoop 阻塞在 recv，会返回 0/EBADF
        fd_ = -1;
    }
}

void Socket::bind(const SocketAddress& addr) {
    if (::bind(fd_, addr.data(), addr.length()) < 0) throwSocketError("bind");
}

void Socket::listen(int backlog) {
    if (::listen(fd_, backlog) < 0) throwSocketError("listen");
}

Socket Socket::accept(SocketAddress* peer) {
    sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    int fd = ::accept(fd_, reinterpret_cast<sockaddr*>(&ss), &len);
    if (fd < 0) throwSocketError("accept");
    if (peer) *peer = SocketAddress::fromSockaddr(reinterpret_cast<sockaddr*>(&ss), len);
    return Socket(fd);
}

void Socket::connect(const SocketAddress& addr) {
    if (::connect(fd_, addr.data(), addr.length()) < 0) throwSocketError("connect");
}

size_t Socket::send(const void* buf, size_t len, int flags) {
    ssize_t n;
    do {
        // MSG_NOSIGNAL：写已关闭的对端不要触发 SIGPIPE
        n = ::send(fd_, buf, len, flags | MSG_NOSIGNAL);
    } while (n < 0 && errno == EINTR);
    if (n < 0) throwSocketError("send");
    return static_cast<size_t>(n);
}

size_t Socket::recv(void* buf, size_t len, int flags) {
    ssize_t n;
    do {
        n = ::recv(fd_, buf, len, flags);
    } while (n < 0 && errno == EINTR);
    if (n < 0) throwSocketError("recv");
    return static_cast<size_t>(n);   // 0 表示对端关闭
}

void Socket::sendAll(const void* buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    size_t left = len;
    while (left > 0) {
        size_t n = send(p, left);
        if (n == 0) throw SocketError("sendAll: connection closed", EPIPE);
        p += n;
        left -= n;
    }
}

void Socket::shutdownWrite() {
    if (::shutdown(fd_, SHUT_WR) < 0) throwSocketError("shutdown");
}

void Socket::setReuseAddr(bool on) {
    int v = on ? 1 : 0;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v)) < 0)
        throwSocketError("setsockopt SO_REUSEADDR");
}

void Socket::setNonBlocking(bool on) {
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags < 0) throwSocketError("fcntl F_GETFL");
    flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (::fcntl(fd_, F_SETFL, flags) < 0) throwSocketError("fcntl F_SETFL");
}

void Socket::setTcpNoDelay(bool on) {
    int v = on ? 1 : 0;
    if (::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v)) < 0)
        throwSocketError("setsockopt TCP_NODELAY");
}

void Socket::setKeepAlive(bool on) {
    int v = on ? 1 : 0;
    if (::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &v, sizeof(v)) < 0)
        throwSocketError("setsockopt SO_KEEPALIVE");
}

// ---------------------------------------------------------------- Buffer
void Buffer::append(const void* data, size_t len) {
    const char* p = static_cast<const char*>(data);
    data_.insert(data_.end(), p, p + len);
}

void Buffer::append(const std::string& s) { append(s.data(), s.size()); }

size_t Buffer::readableBytes() const {
    return data_.size() - readIdx_;
}

const char* Buffer::peek() const {
    return data_.data() + readIdx_;
}

void Buffer::retrieve(size_t len) {
    if (len >= readableBytes()) {
        clear();
    } else {
        readIdx_ += len;
    }
}

std::string Buffer::retrieveAllAsString() {
    std::string s(peek(), readableBytes());
    clear();
    return s;
}

void Buffer::clear() {
    data_.clear();
    readIdx_ = 0;
}

// ---------------------------------------------------------------- 编解码
std::string encodeMessage(const std::string& payload) {
    uint32_t netLen = htonl(static_cast<uint32_t>(payload.size()));
    std::string out;
    out.reserve(sizeof(netLen) + payload.size());
    out.append(reinterpret_cast<const char*>(&netLen), sizeof(netLen));
    out.append(payload);
    return out;
}

bool decodeMessage(Buffer& buf, std::string& msg) {
    if (buf.readableBytes() < sizeof(uint32_t)) return false;

    uint32_t netLen;
    std::memcpy(&netLen, buf.peek(), sizeof(netLen));
    uint32_t len = ntohl(netLen);

    if (len > kMaxMessageSize)
        throw std::runtime_error("decodeMessage: message too large");
    if (buf.readableBytes() < sizeof(uint32_t) + static_cast<size_t>(len))
        return false;

    msg.assign(buf.peek() + sizeof(uint32_t), len);
    buf.retrieve(sizeof(uint32_t) + len);
    return true;
}

// ---------------------------------------------------------------- TcpConnection
TcpConnection::TcpConnection(Socket sock, SocketAddress peer)
    : socket_(std::move(sock)), peer_(std::move(peer)) {}

TcpConnection::~TcpConnection() {
    // 先唤醒可能阻塞在 recv 的读线程
    if (!closed_.exchange(true) && socket_.valid()) {
        ::shutdown(socket_.fd(), SHUT_RDWR);
    }
    if (readThread_.joinable()) {
        if (readThread_.get_id() == std::this_thread::get_id()) {
            // 最后一个 shared_ptr 在读线程回调里释放的情形，join 会死锁
            readThread_.detach();
        } else {
            readThread_.join();
        }
    }
}

void TcpConnection::start() {
    auto self = shared_from_this();
    readThread_ = std::thread([self] { self->readLoop(); });
}

void TcpConnection::send(const void* data, size_t len) {
    std::lock_guard<std::mutex> lk(sendMutex_);
    if (closed_.load())
        throw std::runtime_error("TcpConnection::send: connection closed");
    socket_.sendAll(data, len);
}

void TcpConnection::send(const std::string& s) {
    send(s.data(), s.size());
}

void TcpConnection::close() {
    if (closed_.exchange(true)) return;
    if (socket_.valid()) {
        ::shutdown(socket_.fd(), SHUT_RDWR);   // 打断阻塞 recv
    }
    if (readThread_.joinable() &&
        readThread_.get_id() != std::this_thread::get_id()) {
        readThread_.join();
    }
}

bool TcpConnection::closed() const {
    return closed_.load(std::memory_order_acquire);
}

void TcpConnection::setMessageCallback(MessageCallback cb) {
    onMessage_ = std::move(cb);
}

void TcpConnection::setCloseCallback(CloseCallback cb) {
    onClose_ = std::move(cb);
}

const SocketAddress& TcpConnection::peer() const { return peer_; }

void TcpConnection::readLoop() {
    char tmp[8192];
    try {
        while (!closed_.load()) {
            size_t n = socket_.recv(tmp, sizeof(tmp));
            if (n == 0) break;                 // 对端关闭
            inputBuffer_.append(tmp, n);

            std::string msg;
            while (!closed_.load() && decodeMessage(inputBuffer_, msg)) {
                if (onMessage_) onMessage_(shared_from_this(), msg);
            }
        }
    } catch (const std::exception&) {
        // recv 出错 / 消息超限，统一视为连接断开
    }

    closed_.store(true);
    if (onClose_) onClose_(shared_from_this());
}