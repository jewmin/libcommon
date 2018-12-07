#ifndef __LIBCOMMON_TCP_SERVER_H__
#define __LIBCOMMON_TCP_SERVER_H__

#include <atomic>

#include "list.hpp"
#include "event_loop.h"
#include "tcp_socket.h"
#include "send_packet_pool.h"

class TcpServer : public TcpSocket {
public:
    class TcpConnection;
    friend class TcpConnection;

    void Listen(const char * host, uint16_t port);
    void Shutdown();
    TcpConnection * GetConnection(uint32_t index);

protected:
    TcpServer(EventLoop * loop, const char * name, const uint64_t tick_ms, const int max_out_buffer_size, const int max_in_buffer_size, const int max_free_sockets);
    virtual ~TcpServer();
    void OnConnectFailed() override;
    void OnDisconnected() override;
    TcpSocket * AllocateSocket() override;

    void OnTick();
    virtual void OnTickEvent(TcpConnection * conn) = 0;
    virtual void OnConnectionEstablished(TcpConnection * conn) = 0;
    virtual void OnConnectionClosed(TcpConnection * conn) = 0;
    //virtual void OnReadCompleted(TcpConnection * conn, const char * data, size_t size) = 0;
    virtual void OnReadCompleted(TcpConnection * conn, Packet * packet) = 0;

private:
    void ReleaseSockets();
    void ReleaseSocket(TcpConnection * conn);
    void DestroySocket(TcpConnection * conn);

private:
    bool destroy_;
    uint32_t tick_timer_;
    const uint64_t tick_ms_;
    const int max_free_sockets_;

    typedef TNodeList<TcpConnection> SocketList;
    SocketList active_list_;
    SocketList free_list_;

    static std::atomic<uint32_t> s_num_created_;
};

class TcpServer::TcpConnection : public BaseList::BaseNode, public TcpSocket, public SendPacketPool {
public:
    friend class TcpServer;

    inline uint32_t index() const {
        return index_;
    }

    void Shutdown();

protected:
    TcpConnection(TcpServer & server);
    virtual ~TcpConnection();
    void OnConnected() override;
    void OnConnectFailed() override;
    void OnDisconnected() override;
    //void OnReadCompleted(const char * data, size_t size) override;
    void OnReadCompleted(Packet * packet) override;

private:
    TcpServer & server_;
    const uint32_t index_;
};

#endif