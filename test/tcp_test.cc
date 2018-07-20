#include "gtest/gtest.h"
#include "log_test.h"
#include "tcp_server.h"
#include "tcp_client.h"

TEST(TcpServer, listen_ipv4)
{
    MockLog log;
    TcpServer server("listen_ipv4", 10, 1024, 1024, &log);
    EXPECT_EQ(0, server.Listen("127.0.0.1", 5566));
    server.Stop();
}

TEST(TcpServer, listen_ipv6)
{
    MockLog log;
    TcpServer server("listen_ipv6", 10, 1024, 1024, &log);
    EXPECT_EQ(0, server.Listen("::1", 5566));
    server.Stop();
}

TEST(TcpServer, listen_error)
{
    MockLog log;
    TcpServer server("listen_error", 10, 1024, 1024, &log);
    EXPECT_GT(0, server.Listen("haha", 5566));
}

TEST(TcpServer, accept_success)
{
    MockLog log;
    TcpServer server("accept_success", 10, 1024, 1024, &log);
    server.Listen("0.0.0.0", 5566);

    TcpClient client("accept_success", 10, 1024, 1024, 0, &log);
    client.Connect("127.0.0.1", 5566);

    client.Stop();
    server.Stop();
}

TEST(TcpServer, shutdown_all)
{
    MockLog log;
    TcpServer server("shutdown_all", 10, 1024, 1024, &log);
    server.Listen("0.0.0.0", 5566);

    TcpClient client1("shutdown_all1", 10, 1024, 1024, 0, &log);
    client1.Connect("127.0.0.1", 5566);

    TcpClient client2("shutdown_all2", 10, 1024, 1024, 0, &log);
    client2.Connect("127.0.0.1", 5566);

    server.ShutdownAllConnections();

    server.Stop();
    client1.Stop();
    client2.Stop();
}

TEST(TcpClient, connect_ipv4)
{
    MockLog log;
    TcpServer server("connect_ipv4", 10, 1024, 1024, &log);
    server.Listen("0.0.0.0", 5566);

    TcpClient client("connect_ipv4", 10, 1024, 1024, 0, &log);
    client.Connect("127.0.0.1", 5566);

    server.Stop();
    client.Stop();
}

TEST(TcpClient, connect_ipv6)
{
    MockLog log;
    TcpServer server("connect_ipv6", 10, 1024, 1024, &log);
    server.Listen("::", 5566);

    TcpClient client("connect_ipv6", 10, 1024, 1024, 0, &log);
    client.Connect("::1", 5566);

    server.Stop();
    client.Stop();
}

TEST(TcpClient, connect_fail)
{
    MockLog log;
    TcpServer server("connect_fail", 10, 1024, 1024, &log);
    server.Listen("::", 5566);

    TcpClient client("connect_fail", 10, 1024, 1024, 0, &log);
    client.Connect("haha", 5566);

    server.Stop();
}

TEST(TcpClient, re_connect)
{
    MockLog log;
    TcpServer server("re_connect", 10, 1024, 1024, &log);
    server.Listen("127.0.0.1", 5566);

    TcpClient client("re_connect", 10, 1024, 1024, 100, &log);
    client.Connect("127.0.0.1", 9999);

    Sleep(1000);
    server.Stop();
    client.Stop();
}

TEST(TcpClient, connect_close)
{
    MockLog log;
    TcpServer server("connect_close", 10, 1024, 1024, &log);
    server.Listen("::", 5566);

    TcpClient client("connect_close", 1000, 1024, 1024, 5000, &log);
    client.Connect("127.0.0.1", 5566);

    TcpClient client2("connect_close", 1000, 1024, 1024, 5000, &log);
    client2.Connect("::1", 5566);

    Sleep(10);

    client.Shutdown();
    client2.Shutdown();
    client.Stop();
    client2.Stop();
    server.Stop();
}