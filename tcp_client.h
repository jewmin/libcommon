#ifndef __LIBCOMMON_TCP_CLIENT_H__
#define __LIBCOMMON_TCP_CLIENT_H__

#include "event_loop.h"
#include "tcp_socket.h"
#include "send_packet_pool.h"

class TcpClient : public TcpSocket, public SendPacketPool {
public:
    TcpClient(EventLoop * loop, const char * name, uint64_t tick_ms, const int max_out_buffer_size, const int max_in_buffer_size);
    ~TcpClient();
};

#endif