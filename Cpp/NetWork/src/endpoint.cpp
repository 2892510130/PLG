#include "endpoint.h"

int main()
{
    /*
        1. Create a end point with IP address and port number.
        2. Create a socket:
            2.1 create io context
            2.2 choose the protocal
            2.3 create socket and open it
        3. Create a acceptor (with a buffer length): 
            3.1 [listen ]bind it to a port number so it can listen to all connection to this port number.
            3.2 [accept] Then create another socket for the accept
            3.3 asio provide mutable_buffer (for write) and const_buffer (for read), you can think it's 
                a vector of pointers, each point to a address, the first value is the size.
                We can use buffer() to create a buffer. And for stream we can use streambuf.
        4. socket can connect to a end point.
    */
    // client_end_point();
    // server_end_point();
    // create_tcp_socket();
    dns_connect_to_end();
}

int client_end_point()
{
    std::string raw_ip_address = "127.4.8.1";
    unsigned short port_num = 3333;
    boost::system::error_code ec;
    boost::asio::ip::address ip_address = boost::asio::ip::make_address(raw_ip_address, ec);
    if (ec.value() != 0)
    {
        std::cout << "Failed to parse IP address. Error code = " << ec.value() << ". Message is " << ec.message();
        return ec.value();
    }

    boost::asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

int server_end_point()
{
    boost::asio::ip::address ip_address = boost::asio::ip::address_v6::any();
    unsigned short port_num = 3333;
    boost::asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

int create_tcp_socket()
{
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::socket sock(ioc);
    
    // for new boost impl, we do not need to open
    boost::asio::ip::tcp protocal = boost::asio::ip::tcp::v4();
    boost::system::error_code ec;
    sock.open(protocal, ec);
    if (ec.value() != 0)
    {
        std::cout << "Failed to parse IP address. Error code = " << ec.value() << ". Message is " << ec.message();
        return ec.value();
    }

    return 0;
}

int create_acceptor_socket()
{
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor(ioc);
    
    // And new boost can do this, bind is done too!!
    // boost::asio::ip::tcp::acceptor acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 3333));

    // again, new boost do not need this
    // boost::asio::ip::tcp protocal = boost::asio::ip::tcp::v4();
    // boost::system::error_code ec;
    // acceptor.open(protocal, ec);
    // if (ec.value() != 0)
    // {
    //     std::cout << "Failed to parse IP address. Error code = " << ec.value() << ". Message is " << ec.message();
    //     return ec.value();
    // }

    return 0;
}

int bind_acceptor_socket()
{
    unsigned short port_num = 3333;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
    boost::system::error_code ec;
    acceptor.bind(ep, ec);

    if (ec.value() != 0)
    {
        std::cout << "Failed to open socket. Error code = " << ec.value() << ". Message is " << ec.message();
        return ec.value();
    }

    return 0;
}

int connect_to_end()
{
    std::string raw_ip_address = "192.168.1.124";
    unsigned short port_num = 3333;

    try
    {
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address(raw_ip_address), port_num);
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);
    }
    catch(boost::system::system_error &e)
    {
        std::cout << "Failed to connect. Error code = " << e.code() << ". Message is " << e.what();
        return e.code().value();
    }
    
    return 0;
}

int dns_connect_to_end()
{
    std::string host = "baidu.com";
    std::string port_num = "80";
    boost::asio::io_context ioc;
    boost::asio::ip::tcp::resolver resolver(ioc);

    try
    {
        auto result = resolver.resolve(host, port_num, boost::asio::ip::tcp::resolver::numeric_service);
        boost::asio::ip::tcp::socket sock(ioc);
        boost::asio::connect(sock, result);
        std::cout << "We have connected to baidu.com with port 80!\n";
    }
    catch(boost::system::system_error &e)
    {
        std::cout << "Failed to connect. Error code = " << e.code() << ". Message is " << e.what();
        return e.code().value();
    }

    return 0;
}

int accept_new_connection()
{
    const int BACKLOG_SIZE = 30;
    unsigned short port_num = 80;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);
    boost::asio::io_context ioc;
    
    try
    {
        boost::asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BACKLOG_SIZE);
        boost::asio::ip::tcp::socket sock(ioc);
        acceptor.accept(sock);
    }
    catch(boost::system::system_error &e)
    {
        std::cout << "Failed to connect. Error code = " << e.code() << ". Message is " << e.what();
        return e.code().value();
    }
    
    return 0;
}