#ifndef __UNIT_TEST_SOCKET_SERVER_TEST_H__
#define __UNIT_TEST_SOCKET_SERVER_TEST_H__

#include "gmock/gmock.h"
#include "logger.h"
#include "socket_server.h"
#include "socket_client.h"

class MockSocketServer : public SocketServer
{
public:
    Mutex SocketLock;
    std::list<SocketServer::Socket *> ActiveSocketList;
    bool Listen_done;
    int Accept_Count;
    bool Set_Status;

    static int OnStartAcceptingConnectionsCallCount;
    static int OnStopAcceptingConnectionsCallCount;
    static int OnShutdownInitiatedCallCount;
    static int OnShutdownCompleteCallCount;
    static int OnConnectionCreatedCallCount;
    static int OnConnectionEstablishedCallCount;
    static int OnConnectionClosedCallCount;
    static int OnConnectionDestroyedCallCount;
    static int OnBufferCreatedCallCount;
    static int OnBufferAllocatedCallCount;
    static int OnBufferReleasedCallCount;
    static int OnBufferDestroyedCallCount;
    static int PreWriteCallCount;
    static int ReadCompletedCallCount;
    static int WriteCompletedCallCount;
    static int OnListenFailCallCount;

public:
    MockSocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = NULL);

    virtual ~MockSocketServer();

    virtual void OnStartAcceptingConnections() override;


    virtual void OnStopAcceptingConnections() override;


    virtual void OnShutdownInitiated() override;


    virtual void OnShutdownComplete() override;

    
    virtual void OnListen() override;


    virtual void OnListenFail() override;


    virtual void OnClose() override;


    virtual void OnConnectionCreated() override;


    virtual void OnConnectionEstablished(Socket * socket, Buffer * address) override;


    virtual void OnConnectionClosed(Socket * socket) override;


    virtual void OnConnectionDestroyed() override;


    virtual void OnBufferCreated() override;


    virtual void OnBufferAllocated() override;


    virtual void OnBufferReleased() override;


    virtual void OnBufferDestroyed() override;


    virtual void PreWrite(Socket * socket, Buffer * buffer, const char * data, size_t data_length) override;


    virtual void ReadCompleted(Socket * socket, Buffer * buffer) override;


    virtual void WriteCompleted(Socket * socket, Buffer * buffer, int status) override;

private:
    /*
    * No copies do not implement
    */
    MockSocketServer(const MockSocketServer & rhs);
    MockSocketServer & operator =(const MockSocketServer & rhs);
};

class MockSocketClient : public SocketClient
{
public:
    bool Connect_done;

    static int C_OnStartConnectionsCallCount;
    static int C_OnStopConnectionsCallCount;
    static int C_OnShutdownInitiatedCallCount;
    static int C_OnShutdownCompleteCallCount;
    static int C_OnConnectCallCount;
    static int C_OnConnectFailCallCount;
    static int C_OnCloseCallCount;
    static int C_ReadCompletedCallCount;
    static int C_WriteCompletedCallCount;
    static int C_PreWriteCallCount;

public:
    MockSocketClient(size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = NULL);

    virtual ~MockSocketClient();

    virtual void OnStartConnections() override;


    virtual void OnStopConnections() override;


    virtual void OnShutdownInitiated() override;


    virtual void OnShutdownComplete() override;


    virtual void OnConnect() override;


    virtual void OnConnectFail() override;


    virtual void OnClose() override;


    virtual void ReadCompleted(Buffer * buffer) override;


    virtual void WriteCompleted(Buffer * buffer, int status) override;


    virtual void PreWrite(Buffer * buffer, const char * data, size_t data_length) override;

private:
    /*
    * No copies do not implement
    */
    MockSocketClient(const MockSocketClient & rhs);
    MockSocketClient & operator =(const MockSocketClient & rhs);
};

class MockSocketClient2 : public SocketClient
{
public:
    bool Connect_done;

    static bool bShutdown;

public:
    MockSocketClient2(size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = NULL);

    virtual ~MockSocketClient2();

    virtual void OnConnect() override;

    virtual void OnClose() override;

    virtual void ReadCompleted(Buffer * buffer) override;

    virtual void WriteCompleted(Buffer * buffer, int status) override;

    virtual void PreWrite(Buffer * buffer, const char * data, size_t data_length) override;

private:
    /*
    * No copies do not implement
    */
    MockSocketClient2(const MockSocketClient2 & rhs);
    MockSocketClient2 & operator =(const MockSocketClient2 & rhs);
};

class MockSocketClient3 : public SocketClient
{
public:
    bool Connect_done;

public:
    MockSocketClient3(size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = NULL);

    virtual ~MockSocketClient3();

    virtual void OnConnect() override;

    virtual void ReadCompleted(Buffer * buffer) override;

    virtual void WriteCompleted(Buffer * buffer, int status) override;

private:
    /*
    * No copies do not implement
    */
    MockSocketClient3(const MockSocketClient3 & rhs);
    MockSocketClient3 & operator =(const MockSocketClient3 & rhs);
};

#endif