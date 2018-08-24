#ifndef __UNIT_TEST_SOCKET_SERVER_TEST_H__
#define __UNIT_TEST_SOCKET_SERVER_TEST_H__

#include "gmock/gmock.h"
#include "log.h"
#include "socket_server.h"
#include "socket_client.h"

class MockSocketServer : public SocketServer
{
public:
    Mutex SocketLock;
    std::list<SocketServer::Socket *> ActiveSocketList;

public:
    MockSocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, ILog * logger = NULL);

    virtual ~MockSocketServer();

    virtual void OnStartAcceptingConnections() override;


    virtual void OnStopAcceptingConnections() override;


    virtual void OnShutdownInitiated() override;


    virtual void OnShutdownComplete() override;


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
    MockSocketClient(size_t max_free_buffers, size_t buffer_size = 1024, ILog * logger = NULL);

    virtual ~MockSocketClient();

    bool IsConnected() { return this->GetStatus() == SocketOpt::S_CONNECTED; }

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

#endif