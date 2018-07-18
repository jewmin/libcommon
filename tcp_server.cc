#include "tcp_server.h"
#include "tcp_connection.h"

TcpServer::TcpServer(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger)
    : Super(max_in_buffer_size, logger)
{
    strncpy(this->_name, name, sizeof(this->_name));
    this->_max_out_buffer_size = max_out_buffer_size;
    this->_tick = tick;
    this->_tick_handle.data = this;

    uv_tcp_init(this->_loop, &this->_handle.tcp);
    uv_timer_init(this->_loop, &this->_tick_handle);
}

TcpServer::~TcpServer()
{
    this->_connection_map.clear();
}

int TcpServer::Listen(const char * host, uint16_t port)
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
        r = uv_tcp_bind(&this->_handle.tcp, &s.addr, 0);

    if (r == 0)
        r = uv_listen(&this->_handle.stream, 128, TcpService::ConnectionCallback);

    if (r == 0 && this->_tick > 0)
        r = uv_timer_start(&this->_tick_handle, TcpService::TimerCallback, this->_tick, this->_tick);

    if (r == 0)
        r = this->Start();

    if (r != 0 && this->_logger)
        this->_logger->Error("TCP Server Listen Error: %s", uv_strerror(r));

    return r;
}

TcpConnection * TcpServer::NewConnection(TcpServer & server)
{
    return new TcpConnection(server);
}

void TcpServer::DestroyConnection(TcpConnection * connection)
{
    delete connection;
}

void TcpServer::OnTick()
{
    for (auto & pairs : this->_connection_map)
        pairs.second->OnTick();
}