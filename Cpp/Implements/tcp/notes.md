## Explaination of Code

### SocketAddress

- We use `sockaddr_storage` to store the address because we need to consider both ipv4 and ipv6 (even more other types).
    - Then we use `reinterpret_cast` to cast to `sockaddr_in*` or `sockaddr_in6`
- Set `sin_family`, `AF_INET` is for ipv4
- Need to convert the ip address and port to network byte order (**Big-endian**)
    - For example, 0x12345678, network byte order will put the data in memory address as 0x12, 0x34, 0x56, 0x78.
    - The x86 machine like our machine, use host byte order (**Little-endian**), so we usually need convertion
        - Using `htonl`, `htons` and more functions to do this
- `inet_pton` will convert ip address string presentation to bytes.

## Socket

- Using internal functions like `::socket`, `::bind` and more to do the work
    - A socket is a file descriptor (fd) in Linux, and it contains (our_ip, our_port, there_ip, there_port)
- The kernel will maintain two queue, first is SYN queue and the second is accept queue, so when we call the `::accept` it will just
get one from the accept queue, the `peer` is just to record it.
- After connect or accept we can do send or recv.
- Some configure
    - `setReuseAddr`: Enables address reuse. When a server restarts, the previous process's listening socket may still be in TIME_WAIT; without this, bind fails with EADDRINUSE. Must be called before bind.
    - `setNonBlocking`: Toggles the fd between blocking and non-blocking mode. In non-blocking mode, accept/connect/send/recv return immediately with EAGAIN instead of sleeping, and must be combined with poll/epoll. For connect, non-blocking + poll gives you a timeout; after connecting, you typically switch back to blocking.
    - `setTcpNoDelay`: Disables Nagle's algorithm. Nagle coalesces small packets to reduce packet count, but adds latency. Enable it for interactive, latency-sensitive, small-message workloads; disable it for bulk-throughput transfers.
    - `setKeepAlive`: Enables TCP keepalive probes. Idle connections periodically send probes; if the peer stops responding, the kernel closes the connection so the application can detect dead peers and avoid half-open connections lingering forever. Note the default probe interval is very long (about 2 hours on Linux), so production setups usually also tune TCP_KEEPIDLE/TCP_KEEPINTVL/TCP_KEEPCNT.