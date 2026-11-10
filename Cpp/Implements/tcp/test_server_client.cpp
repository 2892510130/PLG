#include "tcp_server.hpp"
#include "tcp_client.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

static constexpr uint16_t kPort = 19000;

int main() {
    // ---------------------------------------------------------------- server
    TcpServer server(SocketAddress::any(kPort));

    std::atomic<int>  accepted{0};
    std::atomic<int>  closedByServer{0};

    std::mutex              cvMtx;
    std::condition_variable cv;

    server.setConnectionCallback([&](const TcpConnection::Ptr& c) {
        accepted++;
        std::cout << "[server] accepted: " << c->peer().ip()
                  << ":" << c->peer().port() << "\n";
    });

    server.setMessageCallback([&](const TcpConnection::Ptr& c, const std::string& msg) {
        std::cout << "[server] recv: " << msg << "\n";
        c->send(encodeMessage("echo:" + msg));   // 回显
    });

    server.setCloseCallback([&](const TcpConnection::Ptr& c) {
        closedByServer++;
        std::cout << "[server] closed: " << c->peer().ip() << "\n";
        std::lock_guard<std::mutex> lk(cvMtx);
        cv.notify_all();
    });

    std::thread serverThread([&] {
        try { server.start(); }
        catch (const std::exception& e) {
            std::cerr << "[server] start() threw: " << e.what() << "\n";
        }
    });

    // 等监听就绪（简单做法：sleep；要更稳可以循环尝试 connect）
    std::this_thread::sleep_for(100ms);

    // ---------------------------------------------------------------- client
    TcpClient client;

    std::mutex              clientMtx;
    std::condition_variable clientCv;
    std::string             received;
    bool                    clientClosed = false;

    client.setMessageCallback([&](const TcpConnection::Ptr&, const std::string& msg) {
        std::lock_guard<std::mutex> lk(clientMtx);
        received = msg;
        clientCv.notify_all();
    });
    client.setCloseCallback([&](const TcpConnection::Ptr&) {
        std::lock_guard<std::mutex> lk(clientMtx);
        clientClosed = true;
        clientCv.notify_all();
    });

    // 1) connect
    assert(client.connect(SocketAddress::parse("127.0.0.1", kPort), 2000));
    assert(client.connected());
    std::cout << "[client] connected\n";

    // 2) send + 收 echo
    client.send("hello");
    {
        std::unique_lock<std::mutex> lk(clientMtx);
        bool ok = clientCv.wait_for(lk, 2s, [&] { return !received.empty(); });
        assert(ok && "timeout waiting for echo");
        assert(received == "echo:hello");
    }
    std::cout << "[client] got: " << received << "\n";

    // 3) 再发一条，验证解帧循环正常工作
    received.clear();
    client.send("world");
    {
        std::unique_lock<std::mutex> lk(clientMtx);
        bool ok = clientCv.wait_for(lk, 2s, [&] { return !received.empty(); });
        assert(ok && received == "echo:world");
    }
    std::cout << "[client] got: " << received << "\n";

    // 4) client 主动关闭 → server 应收到 onClose
    client.close();
    assert(!client.connected());

    {
        std::unique_lock<std::mutex> lk(cvMtx);
        bool ok = cv.wait_for(lk, 2s, [&] { return closedByServer.load() >= 1; });
        assert(ok && "server didn't see close");
    }
    std::cout << "[test] server saw disconnect OK\n";

    // 5) 多次连接快速验证（accept 循环 + 回调)
    for (int i = 0; i < 3; ++i) {
        TcpClient c2;
        assert(c2.connect(SocketAddress::parse("127.0.0.1", kPort), 2000));
        c2.send("p" + std::to_string(i));
        std::this_thread::sleep_for(20ms);
        c2.close();
    }

    // 6) stop server
    server.stop();
    serverThread.join();
    std::cout << "[test] server stopped\n";

    // 7) stop 之后再连：应直接失败
    TcpClient late;
    bool lateOk = late.connect(SocketAddress::parse("127.0.0.1", kPort), 300);
    assert(!lateOk && "connect to a stopped server should fail");
    std::cout << "[test] late connect rejected as expected\n";

    assert(accepted.load() >= 1);
    std::cout << "\nALL TESTS PASSED\n";
    return 0;
}