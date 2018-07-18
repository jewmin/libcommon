#ifndef __LIB_COMMON_TCP_SERVER_H__
#define __LIB_COMMON_TCP_SERVER_H__

#include "tcp_service.h"
#include <map>

class TcpConnection;
class TcpServer : public TcpService
{
    friend class TcpService;
    friend class TcpConnection;
    typedef TcpService Super;
    typedef std::map<uint32_t, TcpConnection *> ConnectionMap;
    typedef std::map<uint32_t, TcpConnection *>::iterator ConnectionMapIter;

public:
    TcpServer(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpServer();

    int Listen(const char * host, uint16_t port);
    void ShutdownAllConnections();

protected:
    //创建连接函数
    virtual TcpConnection * NewConnection(TcpServer & server);
    //销毁连接函数
    virtual void DestroyConnection(TcpConnection * connection);
    //定时器处理函数
    virtual void OnTick();

protected:
    uv_timer_t _tick_handle;
    ConnectionMap _connection_map;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;

    char _host[256];
    uint16_t _port;
};

#endif