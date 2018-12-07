#ifndef __LIBCOMMON_TCP_CLIENT_H__
#define __LIBCOMMON_TCP_CLIENT_H__

#include "event_loop.h"
#include "tcp_socket.h"
#include "send_packet_pool.h"

class TcpClient : public TcpSocket, public SendPacketPool {
public:
    void Connect(const char * host, uint16_t port);
    void Shutdown();

protected:
    TcpClient(EventLoop * loop, const char * name, const uint64_t tick_ms, const uint64_t reconnect_ms, const int max_out_buffer_size, const int max_in_buffer_size);
    virtual ~TcpClient();
    void OnConnected() override;
    void OnConnectFailed() override;
    void OnDisconnected() override;

    void OnTick();
    virtual void OnTickEvent() = 0;

private:
    bool destroy_;
    uint32_t tick_timer_;
    const uint64_t tick_ms_;
    const uint64_t reconnect_ms_;
};

#endif