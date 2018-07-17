#include "tcp_server.h"

TcpServer::TcpServer(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger)
    : Super(logger)
{
    strncpy(this->_name, name, sizeof(this->_name));
    this->_tick = tick;
    this->_max_in_buffer_size = max_in_buffer_size;
    this->_max_out_buffer_size = max_out_buffer_size;
    this->_tick_handle.data = this;
}

TcpServer::~TcpServer()
{

}

void TcpServer::Listen(const char * host, uint16_t port)
{
    strncpy(this->_host, host, sizeof(this->_host));
    this->_port = port;

    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;

    int r;
    r = uv_ip4_addr(host, port, &s.addr4);
    if (r != 0)
        r = uv_ip6_addr(host, port, &s.addr6);

    if (r == 0)
        r = uv_tcp_init(this->_loop, &this->_handle.tcp);

    if (r == 0)
        r = uv_tcp_bind(&this->_handle.tcp, &s.addr, 0);

    if (r == 0)
        r = uv_listen(&this->_handle.stream, 128, TcpService::ConnectionCallback);

    if (r == 0)
        r = uv_timer_init(this->_loop, &this->_tick_handle);
    
    if (r == 0)
        r = uv_timer_start(&this->_tick_handle, TcpService::TimerCallback, this->_tick, this->_tick);

    if (r == 0)
        r = this->Start();

    if (r != 0 && this->_logger)
        this->_logger->Error("TCP Server Listen Error: %s", uv_strerror(r));
}

void TcpServer::OnClosed()
{

}

void TcpServer::OnTick()
{
    
}

void TcpServer::OnConnected()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Connected");
}

void TcpServer::OnConnectFailed()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Connect Failed");
}

void TcpServer::OnDisconnect()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Disconnect");
}

void TcpServer::OnDisconnected()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Disconnected");
}

void TcpServer::OnRecv(const char * data, int nread)
{
    
}