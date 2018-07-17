#include "tcp_client.h"

TcpClient::TcpClient(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger)
    : Super(logger)
{
    strncpy(this->_name, name, sizeof(this->_name));
    this->_tick = tick;
    this->_max_in_buffer_size = max_in_buffer_size;
    this->_max_out_buffer_size = max_out_buffer_size;
    this->_tick_handle.data = this;
}

TcpClient::~TcpClient()
{

}

void TcpClient::Connect(const char * host, uint16_t port)
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
        r = uv_tcp_connect(&this->_tcp_connect_req, &this->_handle.tcp, &s.addr, TcpService::ConnectCallback);
    
    if (r == 0)
        r = uv_timer_init(this->_loop, &this->_tick_handle);
    
    if (r == 0)
        r = uv_timer_start(&this->_tick_handle, TcpService::TimerCallback, this->_tick, this->_tick);

    if (r == 0)
        r = Super::Start();

    if (r != 0 && this->_logger)
        this->_logger->Error("TCP Client Connect Error: %s", uv_strerror(r));
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
    if (this->_logger)
        this->_logger->Debug("Tcp Server Connected");
}

void TcpClient::OnConnectFailed()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Connect Failed");
}

void TcpClient::OnDisconnect()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Disconnect");
}

void TcpClient::OnDisconnected()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Server Disconnected");
}

void TcpClient::OnRecv(const char * data, int nread)
{
    
}