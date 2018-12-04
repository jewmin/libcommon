#ifndef __UNIT_TEST_SOCKET_TEST_H__
#define __UNIT_TEST_SOCKET_TEST_H__

#include "list.hpp"
#include "tcp_socket.h"
#include "send_packet_pool.h"

const char * hello = "hello world!";
const char * welcome = "welcome to tcp socket.";

class MockClientSocket : public TcpSocket {
public:
    MockClientSocket(EventLoop * loop)
        : TcpSocket(loop, "MockClientSocket", 1024, 1024)
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
        if (log()) {
            log()->LogInfo("%s(%s:%d) client shutdown", name(), GetHost(), GetPort());
        }
        event_loop()->RunInLoop(std::bind(&MockClientSocket::ShutdownInLoop, this));
    }

protected:
    void OnWriteCompleted(int status) {
        if (log()) {
            log()->LogInfo("%s(%s:%d) client write completed (%s)", name(), GetHost(), GetPort(), uv_strerror(status));
        }
    }
    virtual void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) client connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
        Packet packet;
        packet.WriteString(hello);
        SendInLoop(reinterpret_cast<const char *>(packet.GetMemoryPtr()), packet.GetLength(), true, std::bind(&MockClientSocket::OnWriteCompleted, this, std::placeholders::_1));
    }
    virtual void OnConnectFailed() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) client connect failed", name(), GetHost(), GetPort());
        }
        connect_failed_count_++;
        event_loop()->Quit();
    }
    virtual void OnDisconnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) client disconnected", name(), GetHost(), GetPort());
        }
        disconnected_count_++;
        event_loop()->Quit();
    }
    virtual void OnReadCompleted(Packet * packet) override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) client read completed", name(), GetHost(), GetPort());
        }
        read_count_++;
        packet->SetPosition(0);
        const char * read_content = packet->ReadString();
        EXPECT_STREQ(read_content, welcome);
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

    MockServerSocket(EventLoop * loop)
        : TcpSocket(loop, "MockServerSocket", 1024, 1024)
        , connected_count_(0), connect_failed_count_(0), disconnected_count_(0), read_count_(0)
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
            log()->LogInfo("%s(%s:%d) server listen", name(), GetHost(), GetPort());
        }
        connected_count_++;
    }
    virtual void OnConnectFailed() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) server listen failed", name(), GetHost(), GetPort());
        }
        connect_failed_count_++;
    }
    virtual void OnDisconnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) server disconnected", name(), GetHost(), GetPort());
        }
        disconnected_count_++;
    }
    virtual TcpSocket * AllocateSocket() override;
    virtual void OnReadCompleted(Packet * packet) override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) server read completed", name(), GetHost(), GetPort());
        }
        read_count_++;
        packet->SetLength(0);
    }

public:
    int connected_count_;
    int connect_failed_count_;
    int disconnected_count_;
    int read_count_;
    int conn_connected_count_;
    int conn_connect_failed_count_;
    int conn_disconnected_count_;
    int conn_read_count_;
    TNodeList<MockConnection> socket_list_;
};

class MockServerSocket::MockConnection : public BaseList::BaseNode, public TcpSocket {
public:
    MockConnection(MockServerSocket & server)
        : TcpSocket(server.loop_, "MockConnection", 1024, 1024), server_(server) {

    }
    virtual ~MockConnection() {

    }
    void Shutdown() {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connection shutdown", name(), GetHost(), GetPort());
        }
        event_loop()->RunInLoop(std::bind(&MockConnection::ShutdownInLoop, this));
    }

protected:
    void OnWriteCompleted(int status) {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connection write completed (%s)", name(), GetHost(), GetPort(), uv_strerror(status));
        }
    }
    virtual void OnConnected() override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connection connected", name(), GetHost(), GetPort());
        }
        server_.conn_connected_count_++;
        Packet packet;
        packet.WriteString(welcome);
        SendInLoop(reinterpret_cast<const char *>(packet.GetMemoryPtr()), packet.GetLength(), true, std::bind(&MockConnection::OnWriteCompleted, this, std::placeholders::_1));
    }
    virtual void OnConnectFailed() override;
    virtual void OnDisconnected() override;
    virtual void OnReadCompleted(Packet * packet) override {
        if (log()) {
            log()->LogInfo("%s(%s:%d) connection read completed", name(), GetHost(), GetPort());
        }
        server_.conn_read_count_++;
        packet->SetPosition(0);
        const char * read_content = packet->ReadString();
        EXPECT_STREQ(read_content, hello);
        packet->SetLength(0);
        event_loop()->QueueInLoop(std::bind(&MockConnection::ShutdownInLoop, this));
    }

    MockServerSocket & server_;
};

class MockServerSocket_null_conn : public MockServerSocket {
public:
    MockServerSocket_null_conn(EventLoop * loop)
        : MockServerSocket(loop) {

    }
    virtual ~MockServerSocket_null_conn() {

    }

protected:
    virtual TcpSocket * AllocateSocket() override;
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
            log()->LogInfo("%s(%s:%d) client connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
        close_socket();
        Packet packet;
        packet.WriteString(hello);
        SendInLoop(reinterpret_cast<const char *>(packet.GetMemoryPtr()), packet.GetLength(), true, std::bind(&MockClientSocket_write_error::OnWriteCompleted, this, std::placeholders::_1));
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
            log()->LogInfo("%s(%s:%d) client connected", name(), GetHost(), GetPort());
        }
        connected_count_++;
        close_writable();
        Packet packet;
        packet.WriteString(hello);
        SendInLoop(reinterpret_cast<const char *>(packet.GetMemoryPtr()), packet.GetLength(), true, std::bind(&MockClientSocket_write_error2::write_cb, this, std::placeholders::_1));
    }
    void close_writable() {
#ifdef _MSC_VER
        uv_stream()->flags &= ~0x00010000;
#else
        uv_stream()->flags &= ~0x40;
#endif
    }
    void write_cb(int status) {
        if (UV_EPIPE == status) {
            ShutdownInLoop();
        }
    }
};

#endif