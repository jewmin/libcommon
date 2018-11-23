#include "tcp_client.h"

TcpClient::TcpClient(EventLoop * loop, const char * name, uint64_t tick_ms, const int max_out_buffer_size, const int max_in_buffer_size)
    : TcpSocket(loop, name, max_out_buffer_size, max_in_buffer_size), SendPacketPool(this) {

}

TcpClient::~TcpClient() {

}
