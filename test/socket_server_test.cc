#include "gtest/gtest.h"
#include "socket_server_test.h"
#include "log_test.h"

const char * host_ipv4 = "127.0.0.1";
const char * host_ipv6 = "::1";
const char * host_any = "::";
const int port = 6789;
const char content[] = { 'h', 'e', 'l', 'l' , 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', 0 };

int MockSocketServer::OnStartAcceptingConnectionsCallCount = 0;
int MockSocketServer::OnStopAcceptingConnectionsCallCount = 0;
int MockSocketServer::OnShutdownInitiatedCallCount = 0;
int MockSocketServer::OnShutdownCompleteCallCount = 0;
int MockSocketServer::OnConnectionCreatedCallCount = 0;
int MockSocketServer::OnConnectionEstablishedCallCount = 0;
int MockSocketServer::OnConnectionClosedCallCount = 0;
int MockSocketServer::OnConnectionDestroyedCallCount = 0;
int MockSocketServer::OnBufferCreatedCallCount = 0;
int MockSocketServer::OnBufferAllocatedCallCount = 0;
int MockSocketServer::OnBufferReleasedCallCount = 0;
int MockSocketServer::OnBufferDestroyedCallCount = 0;
int MockSocketServer::PreWriteCallCount = 0;
int MockSocketServer::ReadCompletedCallCount = 0;
int MockSocketServer::WriteCompletedCallCount = 0;
int MockSocketServer::OnListenFailCallCount = 0;

int MockSocketClient::C_OnStartConnectionsCallCount = 0;
int MockSocketClient::C_OnStopConnectionsCallCount = 0;
int MockSocketClient::C_OnShutdownInitiatedCallCount = 0;
int MockSocketClient::C_OnShutdownCompleteCallCount = 0;
int MockSocketClient::C_OnConnectCallCount = 0;
int MockSocketClient::C_OnConnectFailCallCount = 0;
int MockSocketClient::C_OnCloseCallCount = 0;
int MockSocketClient::C_ReadCompletedCallCount = 0;
int MockSocketClient::C_WriteCompletedCallCount = 0;
int MockSocketClient::C_PreWriteCallCount = 0;

bool MockSocketClient2::bShutdown = true;

void Reset()
{
    MockSocketServer::OnStartAcceptingConnectionsCallCount = 0;
    MockSocketServer::OnStopAcceptingConnectionsCallCount = 0;
    MockSocketServer::OnShutdownInitiatedCallCount = 0;
    MockSocketServer::OnShutdownCompleteCallCount = 0;
    MockSocketServer::OnConnectionCreatedCallCount = 0;
    MockSocketServer::OnConnectionEstablishedCallCount = 0;
    MockSocketServer::OnConnectionClosedCallCount = 0;
    MockSocketServer::OnConnectionDestroyedCallCount = 0;
    MockSocketServer::OnBufferCreatedCallCount = 0;
    MockSocketServer::OnBufferAllocatedCallCount = 0;
    MockSocketServer::OnBufferReleasedCallCount = 0;
    MockSocketServer::OnBufferDestroyedCallCount = 0;
    MockSocketServer::PreWriteCallCount = 0;
    MockSocketServer::ReadCompletedCallCount = 0;
    MockSocketServer::WriteCompletedCallCount = 0;
    MockSocketServer::OnListenFailCallCount = 0;
}

void C_Reset()
{
    MockSocketClient::C_OnStartConnectionsCallCount = 0;
    MockSocketClient::C_OnStopConnectionsCallCount = 0;
    MockSocketClient::C_OnShutdownInitiatedCallCount = 0;
    MockSocketClient::C_OnShutdownCompleteCallCount = 0;
    MockSocketClient::C_OnConnectCallCount = 0;
    MockSocketClient::C_OnConnectFailCallCount = 0;
    MockSocketClient::C_OnCloseCallCount = 0;
    MockSocketClient::C_ReadCompletedCallCount = 0;
    MockSocketClient::C_WriteCompletedCallCount = 0;
    MockSocketClient::C_PreWriteCallCount = 0;
}

MockSocketServer::MockSocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size /*= 1024*/, Logger * logger /*= NULL*/)
    : SocketServer(max_free_sockets, max_free_buffers, buffer_size, logger)
{
    Listen_done = false;
    Set_Status = false;
    Accept_Count = 0;
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

void MockSocketServer::OnListen()
{
    Listen_done = true;
}

void MockSocketServer::OnListenFail()
{
    OnListenFailCallCount++;
    Listen_done = true;
}

void MockSocketServer::OnClose()
{
    
}

void MockSocketServer::OnConnectionCreated()
{
    OnConnectionCreatedCallCount++;
}

void MockSocketServer::OnConnectionEstablished(Socket * socket, Buffer * address)
{
    OnConnectionEstablishedCallCount++;

    Accept_Count++;

    Mutex::Guard lock(SocketLock);
    ActiveSocketList.push_back(socket);

    if (this->Set_Status)
        socket->Shutdown();
}

void MockSocketServer::OnConnectionClosed(Socket * socket)
{
    OnConnectionClosedCallCount++;

    Mutex::Guard lock(SocketLock);
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

MockSocketClient::MockSocketClient(size_t max_free_buffers, size_t buffer_size /*= 1024*/, Logger * logger /*= NULL*/)
    : SocketClient(max_free_buffers, buffer_size, logger)
{
    Connect_done = false;
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
    Connect_done = true;
    this->Write(content, sizeof(content));
}

void MockSocketClient::OnConnectFail()
{
    C_OnConnectFailCallCount++;
    Connect_done = true;
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

MockSocketClient2::MockSocketClient2(size_t max_free_buffers, size_t buffer_size /*= 1024*/, Logger * logger /*= NULL*/)
    : SocketClient(max_free_buffers, buffer_size, logger)
{
    Connect_done = false;
}

MockSocketClient2::~MockSocketClient2()
{

}

void MockSocketClient2::OnConnect()
{
    Connect_done = true;
    this->Write(content, sizeof(content));
    if (bShutdown) this->Shutdown();
    bShutdown = !bShutdown;
}

void MockSocketClient2::OnClose()
{
    
}

void MockSocketClient2::ReadCompleted(Buffer * buffer)
{

}

void MockSocketClient2::WriteCompleted(Buffer * buffer, int status)
{
    if (bShutdown) this->Shutdown();
}

void MockSocketClient2::PreWrite(Buffer * buffer, const char * data, size_t data_length)
{

}

MockSocketClient3::MockSocketClient3(size_t max_free_buffers, size_t buffer_size /*= 1024*/, Logger * logger /*= NULL*/)
    : SocketClient(max_free_buffers, buffer_size, logger)
{
    Connect_done = false;
}

MockSocketClient3::~MockSocketClient3()
{

}

void MockSocketClient3::OnConnect()
{
    Connect_done = true;
    this->SetStatus(SocketOpt::S_DISCONNECTED);
}

void MockSocketClient3::ReadCompleted(Buffer * buffer)
{

}

void MockSocketClient3::WriteCompleted(Buffer * buffer, int status)
{

}

TEST(SocketTest, init_server)
{
    Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_ipv4, port), 0);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketServer::OnStartAcceptingConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnStopAcceptingConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnShutdownCompleteCallCount, 1);
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
    while (!server.Listen_done) jc_sleep(1);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketServer::OnStartAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnStopAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnShutdownCompleteCallCount, 1);
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
    while (!server.Listen_done) jc_sleep(1);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketServer::OnStartAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnStopAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnShutdownCompleteCallCount, 1);
}

TEST(SocketTest, init_server_error)
{
    Reset();
    MockLog log;

    MockSocketServer server(3, 3, 1024, &log);
    ASSERT_EQ(server.Open("haha", port), 0);
    server.StartAcceptingConnections();
    while (!server.Listen_done) jc_sleep(1);
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketServer::OnStartAcceptingConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnStopAcceptingConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnListenFailCallCount, 1);
}

TEST(SocketTest, init_client)
{
    C_Reset();
    MockLog log;

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    client.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketClient::C_OnStartConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketClient::C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketClient::C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownCompleteCallCount, 1);
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

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 1) jc_sleep(1);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketClient::C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnStopConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnConnectCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnCloseCallCount, 1);

    EXPECT_EQ(MockSocketServer::OnConnectionCreatedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnConnectionEstablishedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnConnectionClosedCallCount, 1);
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

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 1) jc_sleep(1);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketClient::C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnStopConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnConnectCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnCloseCallCount, 1);

    EXPECT_EQ(MockSocketServer::OnConnectionCreatedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnConnectionEstablishedCallCount, 1);
    EXPECT_EQ(MockSocketServer::OnConnectionClosedCallCount, 1);
}

TEST(SocketTest, init_client_error)
{
    C_Reset();
    MockLog log;

    MockSocketClient client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo("haha", port), 0);
    client.StartConnections();

    while (!client.Connect_done) jc_sleep(1);

    client.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketClient::C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketClient::C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnConnectFailCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnCloseCallCount, 0);
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

    while (!server.Listen_done) jc_sleep(1);
    while (!client.Connect_done) jc_sleep(1);

    client.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketClient::C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketClient::C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnConnectFailCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnCloseCallCount, 1);

    EXPECT_EQ(MockSocketServer::OnConnectionCreatedCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnConnectionEstablishedCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnConnectionClosedCallCount, 0);
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

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 1) jc_sleep(1);

    server.WaitForShutdownToComplete();
    client.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketClient::C_PreWriteCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_WriteCompletedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_ReadCompletedCallCount, 1);

    EXPECT_EQ(MockSocketServer::PreWriteCallCount, 1);
    EXPECT_EQ(MockSocketServer::WriteCompletedCallCount, 1);
    EXPECT_EQ(MockSocketServer::ReadCompletedCallCount, 1);
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

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 2) jc_sleep(1);

    EXPECT_GE(server.ActiveSocketList.size(), 1);
    SocketServer::Socket * socket = server.ActiveSocketList.front();
    socket->AbortiveClose();
    client2.StartConnections();

    while (server.Accept_Count < 3) jc_sleep(1);

    client.WaitForShutdownToComplete();
    client1.WaitForShutdownToComplete();
    client2.WaitForShutdownToComplete();
    server.WaitForShutdownToComplete();

    EXPECT_EQ(MockSocketServer::OnConnectionCreatedCallCount, 2);
    EXPECT_EQ(MockSocketServer::OnConnectionEstablishedCallCount, 3);
    EXPECT_EQ(MockSocketServer::OnConnectionClosedCallCount, 3);
    EXPECT_EQ(MockSocketServer::OnConnectionDestroyedCallCount, 2);
}

TEST(SocketTest, buf_error)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(1, 3, 100, &log);
    ASSERT_EQ(server.Open(host_any, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient * client[3];
    client[0] = new MockSocketClient(3, 100, &log);
    client[1] = new MockSocketClient(3, 180, &log);
    client[2] = new MockSocketClient(3, 1024, &log);

    for (int i = 0; i < 3; i++)
    {
        ASSERT_EQ(client[i]->ConnectTo(host_ipv4, port), 0);
        client[i]->StartConnections();
    }

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 3) jc_sleep(1);

    server.WaitForShutdownToComplete();
    for (int i = 0; i < 3; i++)
        client[i]->WaitForShutdownToComplete();

    for (int i = 0; i < 3; i++)
        delete client[i];

    EXPECT_EQ(MockSocketClient::C_PreWriteCallCount, 3);
    EXPECT_EQ(MockSocketClient::C_WriteCompletedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_ReadCompletedCallCount, 0);

    EXPECT_EQ(MockSocketServer::PreWriteCallCount, 1);
    EXPECT_EQ(MockSocketServer::WriteCompletedCallCount, 0);
    EXPECT_EQ(MockSocketServer::ReadCompletedCallCount, 1);
}

TEST(SocketTest, accept_error)
{
    Reset();
    C_Reset();
    MockLog log;

    MockSocketServer server(1, 3, 10, &log);
    ASSERT_EQ(server.Open(host_any, port), 0);
    server.StartAcceptingConnections();

    const int count = 1;
    MockSocketClient * client[count];
    client[0] = new MockSocketClient(3, 100, &log);

    for (int i = 0; i < count; i++)
    {
        ASSERT_EQ(client[i]->ConnectTo(host_ipv4, port), 0);
        client[i]->StartConnections();
    }

    while (!server.Listen_done) jc_sleep(1);
    for (int i = 0; i < count; i++)
        while (!client[i]->Connect_done) jc_sleep(1);

    server.WaitForShutdownToComplete();
    for (int i = 0; i < count; i++)
        client[i]->WaitForShutdownToComplete();

    for (int i = 0; i < count; i++)
        delete client[i];

    EXPECT_EQ(MockSocketClient::C_OnStartConnectionsCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnStopConnectionsCallCount, 0);
    EXPECT_EQ(MockSocketClient::C_OnShutdownInitiatedCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnShutdownCompleteCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnConnectCallCount, 1);
    EXPECT_EQ(MockSocketClient::C_OnCloseCallCount, 1);

    EXPECT_EQ(MockSocketServer::OnConnectionCreatedCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnConnectionEstablishedCallCount, 0);
    EXPECT_EQ(MockSocketServer::OnConnectionClosedCallCount, 0);
}

TEST(SokcetTest, error_test)
{
    MockLog log;

    MockSocketServer server(1, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_any, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient2 client(3, 1024, &log), client2(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    ASSERT_EQ(client2.ConnectTo(host_ipv4, port), 0);
    client.StartConnections();
    client2.StartConnections();

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 2) jc_sleep(1);

    server.WaitForShutdownToComplete();
    client.WaitForShutdownToComplete();
    client2.WaitForShutdownToComplete();
}

TEST(SokcetTest, read_error)
{
    MockLog log;

    MockSocketServer server(1, 3, 1024, &log);
    ASSERT_EQ(server.Open(host_any, port), 0);
    server.StartAcceptingConnections();

    MockSocketClient3 client(3, 1024, &log);
    ASSERT_EQ(client.ConnectTo(host_ipv4, port), 0);
    client.StartConnections();

    while (!server.Listen_done) jc_sleep(1);
    while (server.Accept_Count < 1) jc_sleep(1);

    server.WaitForShutdownToComplete();
    client.WaitForShutdownToComplete();
}