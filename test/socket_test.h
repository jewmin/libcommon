#ifndef __UNIT_TEST_SOCKET_TEST_H__
#define __UNIT_TEST_SOCKET_TEST_H__

#include <list>
#include "tcp_socket.h"
#include "send_packet_pool.h"

static char hello[] = { 'h','e','l','l','o',' ','w','o','r','l','d','!',0 };

class MockClientSocket : public TcpSocket, public SendPacketPool {
public:
    MockClientSocket(EventLoop * loop)
        : TcpSocket(loop, "MockClientSocket", 1024, 1024), SendPacketPool(this)
        , connected_count_(0), connect_failed_count_(0), disconnected_count_(0), read_count_(0) {

    }
    virtual ~MockClientSocket() {

    }
    void Connect(const char * host, uint16_t port) {
        SetHost(host);
        SetPort(port);
        event_loop()->RunInLoop(std::bind(&MockClientSocket::ConnectInLoop, this));
    }
    void Shutdown() {
        event_loop()->RunInLoop(std::bind(&MockClientSocket::ShutdownInLoop, this));
    }

protected:
    virtual void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
        SendToSocket();
    }
    virtual void OnConnectFailed() override {
        connect_failed_count_++;
        loop_->Quit();
    }
    virtual void OnDisconnected() override {
        disconnected_count_++;
        loop_->Quit();
    }
    virtual void OnReadCompleted(Packet * packet) override {
        read_count_++;
        Packet & send_packet = AllocSendPacket();
        send_packet.WriteBinary(packet->GetMemoryPtr(), packet->GetLength());
        Flush(send_packet);
        packet->SetLength(0);
    }

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

    MockServerSocket(EventLoop * loop, int close_after_read_count)
        : TcpSocket(loop, "MockServerSocket", 1024, 1024)
        , close_after_read_count_(close_after_read_count), connected_count_(0), connect_failed_count_(0), disconnected_count_(0), read_count_(0)
        , conn_connected_count_(0), conn_connect_failed_count_(0), conn_disconnected_count_(0), conn_read_count_(0) {

    }
    virtual ~MockServerSocket() {

    }
    void Listen(const char * host, uint16_t port) {
        SetHost(host);
        SetPort(port);
        event_loop()->RunInLoop(std::bind(&MockServerSocket::ListenInLoop, this));
    }
    void Shutdown();

protected:
    virtual void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
    }
    virtual void OnConnectFailed() override {
        connect_failed_count_++;
    }
    virtual void OnDisconnected() override {
        disconnected_count_++;
    }
    virtual TcpSocket * AllocateSocket() override;
    virtual void OnReadCompleted(Packet * packet) override {
        read_count_++;
        packet->SetLength(0);
    }

public:
    int close_after_read_count_;
    int connected_count_;
    int connect_failed_count_;
    int disconnected_count_;
    int read_count_;
    int conn_connected_count_;
    int conn_connect_failed_count_;
    int conn_disconnected_count_;
    int conn_read_count_;
    std::list<MockConnection> socket_list_;
};

class MockServerSocket::MockConnection : public TcpSocket, public SendPacketPool {
public:
    MockConnection(MockServerSocket & server)
        : TcpSocket(server.loop_, "MockConnection", 1024, 1024), SendPacketPool(this)
        , server_(server) {

    }
    virtual ~MockConnection() {

    }
    void Shutdown() {
        event_loop()->RunInLoop(std::bind(&MockConnection::ShutdownInLoop, this));
    }

protected:
    virtual void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connected", name(), GetHost(), GetPort());
        }
        server_.conn_connected_count_++;
        SendInLoop(hello, sizeof(hello));
    }
    virtual void OnConnectFailed() override {
        server_.conn_connect_failed_count_++;
    }
    virtual void OnDisconnected() override {
        server_.conn_disconnected_count_++;
        delete this;
    }
    virtual void OnReadCompleted(Packet * packet) override {
        server_.conn_read_count_++;
        EXPECT_STREQ(reinterpret_cast<const char *>(packet->GetMemoryPtr()), hello);
        EXPECT_EQ(packet->GetLength(), sizeof(hello));
        packet->SetLength(0);
        if (server_.close_after_read_count_ == server_.conn_read_count_) {
            Shutdown();
        }
    }

    MockServerSocket & server_;
};

class MockClientSocket_write_error : public MockClientSocket {
public:
    MockClientSocket_write_error(EventLoop * loop)
        : MockClientSocket(loop) {

    }
    virtual ~MockClientSocket_write_error() {

    }

protected:
    void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
        close_socket();
        Packet & packet = AllocSendPacket();
        packet.WriteString(hello, sizeof(hello));
        Flush(packet);
    }

    void close_socket() {
        uv_os_fd_t fd;

        int r = uv_fileno(uv_handle(), &fd);
#ifdef _MSC_VER
        r = closesocket((uv_os_sock_t)fd);
#else
        r = close(fd);
#endif
    }
};

class MockClientSocket_write_error2 : public MockClientSocket {
public:
    MockClientSocket_write_error2(EventLoop * loop)
        : MockClientSocket(loop) {

    }
    virtual ~MockClientSocket_write_error2() {

    }

protected:
    void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
#ifdef _MSC_VER
        uv_stream()->flags &= ~0x00010000;
#else
        uv_stream()->flags &= ~0x40;
#endif
        SendInLoop(hello, sizeof(hello));
    }
};

#endif