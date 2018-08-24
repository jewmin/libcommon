#include "gtest/gtest.h"
#include "socket_server_test.h"
#include "log_test.h"

int OnStartAcceptingConnectionsCallCount = 0;
int OnStopAcceptingConnectionsCallCount = 0;
int OnShutdownInitiatedCallCount = 0;
int OnShutdownCompleteCallCount = 0;
int OnConnectionCreatedCallCount = 0;
int OnConnectionEstablishedCallCount = 0;
int OnConnectionClosedCallCount = 0;
int OnConnectionDestroyedCallCount = 0;
int OnBufferCreatedCallCount = 0;
int OnBufferAllocatedCallCount = 0;
int OnBufferReleasedCallCount = 0;
int OnBufferDestroyedCallCount = 0;
int PreWriteCallCount = 0;
int ReadCompletedCallCount = 0;
int WriteCompletedCallCount = 0;

const char * host_ipv4 = "127.0.0.1";
const char * host_ipv6 = "::1";
const char * host_any = "::";
const int port = 6789;
const char content[] = { 'h', 'e', 'l', 'l' , 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', 0 };

int C_OnStartConnectionsCallCount = 0;
int C_OnStopConnectionsCallCount = 0;
int C_OnShutdownInitiatedCallCount = 0;
int C_OnShutdownCompleteCallCount = 0;
int C_OnConnectCallCount = 0;
int C_OnConnectFailCallCount = 0;
int C_OnCloseCallCount = 0;
int C_ReadCompletedCallCount = 0;
int C_WriteCompletedCallCount = 0;
int C_PreWriteCallCount = 0;

void Reset()
{
    OnStartAcceptingConnectionsCallCount = 0;
    OnStopAcceptingConnectionsCallCount = 0;
    OnShutdownInitiatedCallCount = 0;
    OnShutdownCompleteCallCount = 0;
    OnConnectionCreatedCallCount = 0;
    OnConnectionEstablishedCallCount = 0;
    OnConnectionClosedCallCount = 0;
    OnConnectionDestroyedCallCount = 0;
    OnBufferCreatedCallCount = 0;
    OnBufferAllocatedCallCount = 0;
    OnBufferReleasedCallCount = 0;
    OnBufferDestroyedCallCount = 0;
    PreWriteCallCount = 0;
    ReadCompletedCallCount = 0;
    WriteCompletedCallCount = 0;
}

void C_Reset()
{
    C_OnStartConnectionsCallCount = 0;
    C_OnStopConnectionsCallCount = 0;
    C_OnShutdownInitiatedCallCount = 0;
    C_OnShutdownCompleteCallCount = 0;
    C_OnConnectCallCount = 0;
    C_OnConnectFailCallCount = 0;
    C_OnCloseCallCount = 0;
    C_ReadCompletedCallCount = 0;
    C_WriteCompletedCallCount = 0;
    C_PreWriteCallCount = 0;
}

MockSocketServer::MockSocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size /*= 1024*/, ILog * logger /*= NULL*/)
    : SocketServer(max_free_sockets, max_free_buffers, buffer_size, logger)
{

}

MockSocketServer::~MockSocketServer()
{
    ReleaseSockets();
    ReleaseBuffers();
}

void MockSocketServer::OnStartAcceptingConnections()
{
    OnStartAcceptingConnectionsCallCount++;
}

void MockSocketServer::OnStopAcceptingConnections()
{
    OnStopAcceptingConnectionsCallCount++;
}

void MockSocketServer::OnShutdownInitiated()
{
    OnShutdownInitiatedCallCount++;
}

void MockSocketServer::OnShutdownComplete()
{
    OnShutdownCompleteCallCount++;
}

void MockSocketServer::OnConnectionCreated()
{
    OnConnectionCreatedCallCount++;
}

void MockSocketServer::OnConnectionEstablished(Socket * socket, Buffer * address)
{
    OnConnectionEstablishedCallCount++;

    Mutex::Owner lock(SocketLock);
    ActiveSocketList.push_back(socket);
}

void MockSocketServer::OnConnectionClosed(Socket * socket)
{
    OnConnectionClosedCallCount++;

    Mutex::Owner lock(SocketLock);
    ActiveSocketList.remove(socket);
}

void MockSocketServer::OnConnectionDestroyed()
{
    OnConnectionDestroyedCallCount++;
}

void MockSocketServer::OnBufferCreated()
{
    OnBufferCreatedCallCount++;
}

void MockSocketServer::OnBufferAllocated()
{
    OnBufferAllocatedCallCount++;
}

void MockSocketServer::OnBufferReleased()
{
    OnBufferReleasedCallCount++;
}

void MockSocketServer::OnBufferDestroyed()
{
    OnBufferDestroyedCallCount++;
}

void MockSocketServer::PreWrite(Socket * socket, Buffer * buffer, const char * data, size_t data_length)
{
    PreWriteCallCount++;
}

void MockSocketServer::ReadCompleted(Socket * socket, Buffer * buffer)
{
    ReadCompletedCallCount++;
    Buffer * message_buffer = buffer->SplitBuffer(buffer->GetUsed());
    socket->Write(message_buffer, ReadCompletedCallCount % 2 == 0);
    socket->Write((const char *)message_buffer->GetBuffer(), message_buffer->GetUsed(), ReadCompletedCallCount % 2 == 0);
    message_buffer->Release();
    socket->Read(buffer);
}

void MockSocketServer::WriteCompleted(Socket * socket, Buffer * buffer, int status)
{
    WriteCompletedCallCount++;
}

MockSocketClient::MockSocketClient(size_t max_free_buffers, size_t buffer_size /*= 1024*/, ILog * logger /*= NULL*/)
    : SocketClient(max_free_buffers, buffer_size, logger)
{

}

MockSocketClient::~MockSocketClient()
{

}

void MockSocketClient::OnStartConnections()
{
    C_OnStartConnectionsCallCount++;
}

void MockSocketClient::OnStopConnections()
{
    C_OnStopConnectionsCallCount++;
}

void MockSocketClient::OnShutdownInitiated()
{
    C_OnShutdownInitiatedCallCount++;
}

void MockSocketClient::OnShutdownComplete()
{
    C_OnShutdownCompleteCallCount++;
}

void MockSocketClient::OnConnect()
{
    C_OnConnectCallCount++;
    this->Write(content, sizeof(content));
}

void MockSocketClient::OnConnectFail()
{
    C_OnConnectFailCallCount++;
}

void MockSocketClient::OnClose()
{
    C_OnCloseCallCount++;
}

void MockSocketClient::ReadCompleted(Buffer * buffer)
{
    C_ReadCompletedCallCount++;
    buffer->Empty();
    this->Read(buffer);
}

void MockSocketClient::WriteCompleted(Buffer * buffer, int status)
{
    C_WriteCompletedCallCount++;
}

void MockSocketClient::PreWrite(Buffer * buffer, const char * data, size_t data_length)
{
    C_PreWriteCallCount++;
}

TEST(SocketTest, init_server)
{
    Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv4, port), 0);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(OnStartAcceptingConnectionsCallCount, 0);
    EXPECT_EQ(OnStopAcceptingConnectionsCallCount, 0);
    EXPECT_EQ(OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(OnShutdownCompleteCallCount, 1);
}

TEST(SocketTest, init_server_ipv4)
{
    Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv4, port), 0);
    server.SetNoDelay(true);
    server.SetKeepAlive(60);
    server.StartAcceptingConnections();
    jc_sleep(2000);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(OnStartAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(OnStopAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(OnShutdownCompleteCallCount, 1);
}

TEST(SocketTest, init_server_ipv6)
{
    Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv6, port), 0);
    server.SetNoDelay(false);
    server.SetKeepAlive(0);
    server.StartAcceptingConnections();
    jc_sleep(2000);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(OnStartAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(OnStopAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(OnShutdownCompleteCallCount, 1);
}

TEST(SocketTest, init_server_error)
{
    Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open("haha", port), 0);
    server.StartAcceptingConnections();
    jc_sleep(2000);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(OnStartAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(OnStopAcceptingConnectionsCallCount, 0);
    EXPECT_EQ(OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(OnShutdownCompleteCallCount, 1);
}

TEST(SocketTest, init_client)
{
    C_Reset();
    MockLog log;

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    client.WaitForShutdownToComplete();

    EXPECT_EQ(C_OnStartConnectionsCallCount, 0);
    EXPECT_EQ(C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(C_OnShutdownCompleteCallCount, 1);
}

TEST(SocketTest, init_client_ipv4)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv4, port), 0);
    server.SetNoDelay(true);
    server.SetKeepAlive(60);
    server.StartAcceptingConnections();

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    client.SetNoDelay(true);
    client.SetKeepAlive(60);
    client.StartConnections();

    jc_sleep(2000);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(C_OnStopConnectionsCallCount, 1);
    EXPECT_EQ(C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(C_OnConnectCallCount, 1);
    EXPECT_EQ(C_OnCloseCallCount, 1);

    EXPECT_EQ(OnConnectionCreatedCallCount, 1);
    EXPECT_EQ(OnConnectionEstablishedCallCount, 1);
    EXPECT_EQ(OnConnectionClosedCallCount, 1);
}

TEST(SocketTest, init_client_ipv6)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv6, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv6, port), 0);
    client.SetNoDelay(false);
    client.SetKeepAlive(0);
    client.StartConnections();

    jc_sleep(2000);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(C_OnStopConnectionsCallCount, 1);
    EXPECT_EQ(C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(C_OnConnectCallCount, 1);
    EXPECT_EQ(C_OnCloseCallCount, 1);

    EXPECT_EQ(OnConnectionCreatedCallCount, 1);
    EXPECT_EQ(OnConnectionEstablishedCallCount, 1);
    EXPECT_EQ(OnConnectionClosedCallCount, 1);
}

TEST(SocketTest, init_client_error)
{
    C_Reset();
    MockLog log;

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo("haha", port), 0);
    client.StartConnections();

    jc_sleep(2000);

    client.WaitForShutdownToComplete();

    EXPECT_EQ(C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(C_OnConnectFailCallCount, 0);
    EXPECT_EQ(C_OnCloseCallCount, 0);
}

TEST(SocketTest, init_client_fail)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv4, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv6, port), 0);
    client.StartConnections();

    jc_sleep(2000);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(C_OnConnectFailCallCount, 1);
    EXPECT_EQ(C_OnCloseCallCount, 1);

    EXPECT_EQ(OnConnectionCreatedCallCount, 0);
    EXPECT_EQ(OnConnectionEstablishedCallCount, 0);
    EXPECT_EQ(OnConnectionClosedCallCount, 0);
}

TEST(SocketTest, write_data)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_any, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    client.StartConnections();

    jc_sleep(2000);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(C_PreWriteCallCount, 1);
    EXPECT_EQ(C_WriteCompletedCallCount, 1);
    EXPECT_EQ(C_ReadCompletedCallCount, 1);

    EXPECT_EQ(PreWriteCallCount, 1);
    EXPECT_EQ(WriteCompletedCallCount, 1);
    EXPECT_EQ(ReadCompletedCallCount, 1);
}

TEST(SocketTest, close_socket)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_any, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient client(3, 1024, &log), client1(3, 1024, &log), client2(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    ASSERT_EQ(client1.ConnectTo(host_ipv6, port), 0);
    ASSERT_EQ(client2.ConnectTo(host_ipv6, port), 0);
    client.StartConnections();
    client1.StartConnections();

    jc_sleep(2000);

    EXPECT_GE(server.ActiveSocketList.size(), 1);
    SocketServer::Socket * socket = server.ActiveSocketList.front();
    socket->AbortiveClose();
    client2.StartConnections();

    jc_sleep(2000);

    client.WaitForShutdownToComplete();
    client1.WaitForShutdownToComplete();
    client2.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(OnConnectionCreatedCallCount, 2);
    EXPECT_EQ(OnConnectionEstablishedCallCount, 3);
    EXPECT_EQ(OnConnectionClosedCallCount, 3);
    EXPECT_EQ(OnConnectionDestroyedCallCount, 2);
}