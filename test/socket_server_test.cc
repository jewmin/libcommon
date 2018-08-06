#include "gtest/gtest.h"
#include "socket_server_test.h"
#include "common.h"

//TEST(SocketServerTest, socket_close)
//{
//    MockSocketServer server(0, 0);
//    server.Open("::", 6789);
//    server.StartAcceptingConnections();
//    while (!server._last_socket) jc_sleep(100);
//    server.Close();
//    jc_sleep(1000);
//    server.WaitForShutdownToComplete();
//}

TEST(SocketServerTest, listen_error)
{
    MockSocketServer server(0, 0);
    server.Open("haha", 6789);
    server.StartAcceptingConnections();
    jc_sleep(10);
    server.WaitForShutdownToComplete();
}