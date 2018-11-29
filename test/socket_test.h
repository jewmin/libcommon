#ifndef __UNIT_TEST_SOCKET_TEST_H__
#define __UNIT_TEST_SOCKET_TEST_H__

#include <list>
#include "tcp_socket.h"
#include "send_packet_pool.h"

class MockClientSocket : public TcpSocket, public SendPacketPool {
public:
    MockClientSocket(EventLoop * loop);
    virtual ~MockClientSocket();
    void Connect(const char * host, uint16_t port);
    void Shutdown();

protected:
    virtual void OnConnected() override;
    virtual void OnConnectFailed() override;
    virtual void OnDisconnected() override;
    virtual void OnReadCompleted(const char * data, size_t size) override;

public:
    int connected_count_;
    int connect_failed_count_;
    int disconnected_count_;
    int read_count_;
};

class MockServerSocket : public TcpSocket {
public:
    class MockConnection;
    friend class MockConnection;

    MockServerSocket(EventLoop * loop, int close_after_read_count);
    virtual ~MockServerSocket();
    void Listen(const char * host, uint16_t port);
    void Shutdown();

protected:
    virtual void OnConnected() override;
    virtual void OnConnectFailed() override;
    virtual void OnDisconnected() override;
    virtual void OnReadCompleted(const char * data, size_t size) override;
    virtual TcpSocket * AllocateSocket() override;

public:
    int close_after_read_count_;
    int connected_count_;
    int connect_failed_count_;
    int disconnected_count_;
    int read_count_;
    std::list<MockConnection> socket_list_;
};

class MockServerSocket::MockConnection : public TcpSocket, public SendPacketPool {
public:
    MockConnection(MockServerSocket & server);
    virtual ~MockConnection();
    void Shutdown();

protected:
    virtual void OnConnected() override;
    virtual void OnConnectFailed() override;
    virtual void OnDisconnected() override;
    virtual void OnReadCompleted(const char * data, size_t size) override;

public:
    int connected_count_;
    int connect_failed_count_;
    int disconnected_count_;
    int read_count_;
    MockServerSocket & server_;
};

#endif