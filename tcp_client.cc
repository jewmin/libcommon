#include "tcp_client.h"

TcpClient::TcpClient(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, uint32_t reconnect_tick, ILog * logger)
    : Super(max_in_buffer_size, logger)
{
    strncpy(this->_name, name, sizeof(this->_name) - 1);
    this->_tick = tick;
    this->_max_out_buffer_size = max_out_buffer_size;
    this->_reconnect_tick = reconnect_tick;
    this->_tick_handle.data = this;
    this->_reconnect_tick_handle.data = this;

    uv_tcp_init(this->_loop, &this->_handle.tcp);
    uv_timer_init(this->_loop, &this->_tick_handle);
    uv_timer_init(this->_loop, &this->_reconnect_tick_handle);
}

TcpClient::~TcpClient()
{

}

int TcpClient::Connect(const char * host, uint16_t port)
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
        r = uv_tcp_connect(&this->_tcp_connect_req, &this->_handle.tcp, &s.addr, TcpService::ConnectCallback);

    if (r == 0)
        r = uv_send_buffer_size(&this->_handle.handle, (int *)&this->_max_out_buffer_size);

    if (r == 0)
        r = uv_recv_buffer_size(&this->_handle.handle, (int *)&this->_max_in_buffer_size);
    
    if (r == 0 && this->_tick > 0)
        r = uv_timer_start(&this->_tick_handle, TcpService::TimerCallback, this->_tick, this->_tick);

    if (r == 0)
        r = Super::Start();

    if (r != 0 && this->_logger)
        this->_logger->Error("%s Connect Error: %s", this->GetName(), uv_strerror(r));

    return r;
}

int TcpClient::ReConnect()
{
    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;

    int r;
    r = uv_ip4_addr(this->_host, this->_port, &s.addr4);
    if (r != 0)
        r = uv_ip6_addr(this->_host, this->_port, &s.addr6);

    if (r == 0)
        r = uv_tcp_connect(&this->_tcp_connect_req, &this->_handle.tcp, &s.addr, TcpService::ConnectCallback);

    if (r == 0)
        r = uv_send_buffer_size(&this->_handle.handle, (int *)&this->_max_out_buffer_size);

    if (r == 0)
        r = uv_recv_buffer_size(&this->_handle.handle, (int *)&this->_max_in_buffer_size);
    
    if (r == 0 && this->_tick > 0)
        r = uv_timer_start(&this->_tick_handle, TcpService::TimerCallback, this->_tick, this->_tick);

    if (r != 0 && this->_logger)
        this->_logger->Error("%s Connect Error: %s", this->GetName(), uv_strerror(r));

    return r;
}

void TcpClient::OnClosing()
{
    this->OnDisconnect();
}

void TcpClient::OnClosed()
{
    this->OnDisconnected();
}

void TcpClient::OnTick()
{

}

void TcpClient::OnConnected()
{

}

void TcpClient::OnConnectFailed()
{
    if (this->_reconnect_tick > 0)
        uv_timer_start(&this->_reconnect_tick_handle, TcpClient::ReConnectTimerCallback, this->_reconnect_tick, 0);
}

void TcpClient::OnDisconnect()
{

}

void TcpClient::OnDisconnected()
{

}

void TcpClient::OnRecv(const char * data, int nread)
{

}

void TcpClient::ReConnectTimerCallback(uv_timer_t * handle)
{
    TcpClient * client = (TcpClient *)handle->data;
    client->ReConnect();
}