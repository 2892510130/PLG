#include "tcp_basic.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>

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

const sockaddr* SocketAddress::data() const {
    return reinterpret_cast<const sockaddr*>(&addr_);
}

socklen_t SocketAddress::length() const {
    return len_;
}

std::string SocketAddress::ip() const {
    char buf[INET6_ADDRSTRLEN];
    switch (addr_.ss_family) {
    case AF_INET: {
        const auto* sin = reinterpret_cast<const sockaddr_in*>(&addr_);
        inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
        return buf;
    }
    case AF_INET6: {
        const auto* sin6 = reinterpret_cast<const sockaddr_in6*>(&addr_);
        inet_ntop(AF_INET6, &sin6->sin6_addr, buf, sizeof(buf));
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