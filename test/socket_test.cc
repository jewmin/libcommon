#include "gtest/gtest.h"
#include "socket_test.h"

static char ipv4[] = "127.0.0.1";
static char ipv6[] = "::1";
static char ipv4_any[] = "0.0.0.0";
static char ipv6_any[] = "::";
uint16_t port = 8888;

void MockServerSocket::Shutdown() {
    for (auto & it : socket_list_) {
        it.Shutdown();
    }
    event_loop()->RunInLoop(std::bind(&MockServerSocket::ShutdownInLoop, this));
}

TcpSocket * MockServerSocket::AllocateSocket() {
    return new MockConnection(*this);
}

TcpSocket * MockServerSocket_null_conn::AllocateSocket() {
    return nullptr;
}

TEST(SocketTest, ipv4)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket client(loop);
    server.Listen(ipv4_any, port);
    client.Connect(ipv4, port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 1);
    EXPECT_EQ(client.connect_failed_count_, 0);
    EXPECT_EQ(client.disconnected_count_, 1);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 0);
    EXPECT_EQ(server.disconnected_count_, 1);
    EXPECT_EQ(server.read_count_, 0);

    EXPECT_EQ(server.conn_connected_count_, 1);
    EXPECT_EQ(server.conn_connect_failed_count_, 0);
    EXPECT_EQ(server.conn_disconnected_count_, 1);
    EXPECT_EQ(server.conn_read_count_, 1);
}

TEST(SocketTest, ipv6)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket client(loop);
    server.Listen(ipv6_any, port);
    client.Connect(ipv6, port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 1);
    EXPECT_EQ(client.connect_failed_count_, 0);
    EXPECT_EQ(client.disconnected_count_, 1);
    EXPECT_EQ(client.read_count_, 1);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 0);
    EXPECT_EQ(server.disconnected_count_, 1);
    EXPECT_EQ(server.read_count_, 0);

    EXPECT_EQ(server.conn_connected_count_, 1);
    EXPECT_EQ(server.conn_connect_failed_count_, 0);
    EXPECT_EQ(server.conn_disconnected_count_, 1);
    EXPECT_EQ(server.conn_read_count_, 1);
}

TEST(SocketTest, ip_error)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket client(loop);
    server.Listen("www.baidu.com", port);
    client.Connect("www.baidu.com", port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 0);
    EXPECT_EQ(client.connect_failed_count_, 1);
    EXPECT_EQ(client.disconnected_count_, 0);
    EXPECT_EQ(client.read_count_, 0);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 1);
    EXPECT_EQ(server.disconnected_count_, 0);
    EXPECT_EQ(server.read_count_, 0);

    EXPECT_EQ(server.conn_connected_count_, 0);
    EXPECT_EQ(server.conn_connect_failed_count_, 0);
    EXPECT_EQ(server.conn_disconnected_count_, 0);
    EXPECT_EQ(server.conn_read_count_, 0);
}

TEST(SocketTest, repeat)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket client(loop);
    server.Listen(ipv6_any, port);
    client.Connect(ipv4, port);
    server.Listen(ipv4_any, port);
    client.Connect(ipv6, port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 1);
    EXPECT_EQ(client.connect_failed_count_, 0);
    EXPECT_EQ(client.disconnected_count_, 1);
    EXPECT_EQ(client.read_count_, 1);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 0);
    EXPECT_EQ(server.disconnected_count_, 1);
    EXPECT_EQ(server.read_count_, 0);

    EXPECT_EQ(server.conn_connected_count_, 1);
    EXPECT_EQ(server.conn_connect_failed_count_, 0);
    EXPECT_EQ(server.conn_disconnected_count_, 1);
    EXPECT_EQ(server.conn_read_count_, 1);
}

TEST(SocketTest, write_error)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket_write_error client(loop);
    server.Listen(ipv6_any, port);
    client.Connect(ipv4, port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 1);
    EXPECT_EQ(client.connect_failed_count_, 0);
    EXPECT_EQ(client.disconnected_count_, 1);
    EXPECT_EQ(client.read_count_, 0);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 0);
    EXPECT_EQ(server.disconnected_count_, 1);
    EXPECT_EQ(server.read_count_, 0);

    EXPECT_EQ(server.conn_connected_count_, 1);
    EXPECT_EQ(server.conn_connect_failed_count_, 0);
    EXPECT_EQ(server.conn_disconnected_count_, 1);
    EXPECT_EQ(server.conn_read_count_, 0);
}

TEST(SocketTest, write_error2)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket server(loop, 1);
    MockClientSocket_write_error2 client(loop);
    server.Listen(ipv6_any, port);
    client.Connect(ipv4, port);
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 1);
    EXPECT_EQ(client.connect_failed_count_, 0);
    EXPECT_EQ(client.disconnected_count_, 1);
    EXPECT_EQ(client.read_count_, 0);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 0);
    EXPECT_EQ(server.disconnected_count_, 1);
    EXPECT_EQ(server.read_count_, 0);

    EXPECT_EQ(server.conn_connected_count_, 1);
    EXPECT_EQ(server.conn_connect_failed_count_, 0);
    EXPECT_EQ(server.conn_disconnected_count_, 0);
    EXPECT_EQ(server.conn_read_count_, 0);
}

static void close_cb(uv_handle_t * handle) {
    TcpSocket * socket = static_cast<TcpSocket *>(handle->data);
    socket->event_loop()->Quit();
}

static void timer1_cb(MockClientSocket * client) {
    uv_close(client->uv_handle(), close_cb);
}

TEST(SocketTest, close_while_connecting)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockClientSocket client(loop);
    client.Connect("1.2.3.4", port);
    loop->RunAfter(1, std::bind(&timer1_cb, &client));
    loop->Loop();
    client.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 0);
    EXPECT_EQ(client.connect_failed_count_, 1);
    EXPECT_EQ(client.disconnected_count_, 0);
    EXPECT_EQ(client.read_count_, 0);
}

static void timer2_cb(MockClientSocket * client) {
    client->event_loop()->Quit();
}

TEST(SocketTest, accept_null)
{
    Logger logger;
    logger.InitLogger(Logger::Trace);
    EventLoop * loop = new EventLoop(&logger);
    MockServerSocket_null_conn server(loop);
    MockClientSocket client(loop);
    server.Listen(ipv6_any, port);
    client.Connect(ipv4, port);
    loop->RunAfter(1, std::bind(&timer2_cb, &client));
    loop->Loop();
    client.Shutdown();
    server.Shutdown();
    delete loop;
    logger.DeInitLogger();

    EXPECT_EQ(client.connected_count_, 1);
    EXPECT_EQ(client.connect_failed_count_, 0);
    EXPECT_EQ(client.disconnected_count_, 1);
    EXPECT_EQ(client.read_count_, 0);

    EXPECT_EQ(server.connected_count_, 0);
    EXPECT_EQ(server.connect_failed_count_, 0);
    EXPECT_EQ(server.disconnected_count_, 1);
    EXPECT_EQ(server.read_count_, 0);
}