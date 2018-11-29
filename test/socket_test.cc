#include "gtest/gtest.h"
#include "socket_test.h"

static char hello[] = { 'h','e','l','l','o',' ','w','o','r','l','d','!',0 };
static char ipv4[] = "127.0.0.1";
static char ipv6[] = "::1";
static char ipv4_any[] = "0.0.0.0";
static char ipv6_any[] = "::";
uint16_t port = 8888;

MockClientSocket::MockClientSocket(EventLoop * loop)
    : TcpSocket(loop, "MockClientSocket", 1024, 1024), SendPacketPool(this)
    , connected_count_(0), connect_failed_count_(0), disconnected_count_(0), read_count_(0)
{

}

MockClientSocket::~MockClientSocket()
{

}

void MockClientSocket::Connect(const char * host, uint16_t port)
{
    SetHost(host);
    SetPort(port);
    event_loop()->RunInLoop(std::bind(&MockClientSocket::ConnectInLoop, this));
}

void MockClientSocket::Shutdown()
{
    event_loop()->RunInLoop(std::bind(&MockClientSocket::ShutdownInLoop, this));
}

void MockClientSocket::OnConnected()
{
    connected_count_++;
    SendToSocket();
}

void MockClientSocket::OnConnectFailed()
{
    connect_failed_count_++;
    loop_->Quit();
}

void MockClientSocket::OnDisconnected()
{
    disconnected_count_++;
    loop_->Quit();
}

void MockClientSocket::OnReadCompleted(const char * data, size_t size)
{
    read_count_++;
    Packet & packet = AllocSendPacket();
    packet.WriteBinary(reinterpret_cast<const uint8_t *>(data), size);
    Flush(packet);
}

MockServerSocket::MockServerSocket(EventLoop * loop, int close_after_read_count)
    : TcpSocket(loop, "MockServerSocket", 1024, 1024)
    , close_after_read_count_(close_after_read_count), connected_count_(0), connect_failed_count_(0), disconnected_count_(0), read_count_(0)
{

}

MockServerSocket::~MockServerSocket()
{

}

void MockServerSocket::Listen(const char * host, uint16_t port)
{
    SetHost(host);
    SetPort(port);
    event_loop()->RunInLoop(std::bind(&MockServerSocket::ListenInLoop, this));
}

void MockServerSocket::Shutdown()
{
    for (auto & it : socket_list_) {
        it.Shutdown();
    }
    event_loop()->RunInLoop(std::bind(&MockServerSocket::ShutdownInLoop, this));
}

void MockServerSocket::OnConnected()
{
    connected_count_++;
}

void MockServerSocket::OnConnectFailed()
{
    connect_failed_count_++;
}

void MockServerSocket::OnDisconnected()
{
    disconnected_count_++;
}

void MockServerSocket::OnReadCompleted(const char * data, size_t size)
{
    read_count_++;
}

TcpSocket * MockServerSocket::AllocateSocket()
{
    return new MockConnection(*this);
}

MockServerSocket::MockConnection::MockConnection(MockServerSocket & server)
    : TcpSocket(server.loop_, "MockConnection", 1024, 1024), SendPacketPool(this)
    , connected_count_(0), connect_failed_count_(0), disconnected_count_(0), read_count_(0)
    , server_(server)

{

}

MockServerSocket::MockConnection::~MockConnection()
{

}

void MockServerSocket::MockConnection::Shutdown()
{
    event_loop()->RunInLoop(std::bind(&MockConnection::ShutdownInLoop, this));
}

void MockServerSocket::MockConnection::OnConnected()
{
    connected_count_++;
    SendInLoop(hello, sizeof(hello));
}

void MockServerSocket::MockConnection::OnConnectFailed()
{
    connect_failed_count_++;
}

void MockServerSocket::MockConnection::OnDisconnected()
{
    disconnected_count_++;
    delete this;
}

void MockServerSocket::MockConnection::OnReadCompleted(const char * data, size_t size)
{
    read_count_++;
    EXPECT_STREQ(data, hello);
    EXPECT_EQ(size, sizeof(hello));
    if (server_.close_after_read_count_ == read_count_) {
        Shutdown();
    }
}

TEST(SocketTest, ipv4)
{
    Logger logger;
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket client(loop);
    server.Listen(ipv4_any, port);
    client.Connect(ipv4, port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
}