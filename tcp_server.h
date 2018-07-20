#ifndef __LIB_COMMON_TCP_SERVER_H__
#define __LIB_COMMON_TCP_SERVER_H__

#include "tcp_service.h"
#include <set>

class TcpConnection;
class TcpServer : public TcpService
{
    friend class TcpService;
    friend class TcpConnection;
    typedef TcpService Super;
    typedef std::set<TcpConnection *> ConnectionSet;
    typedef std::set<TcpConnection *>::iterator ConnectionSetIter;

    //TCP消息
    enum TcpMsg
    {
        eShutdownAll = 1, //关闭所有连接
    };

public:
    TcpServer(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpServer();

    int Listen(const char * host, uint16_t port);
    void ShutdownAllConnections();

protected:
    void AddConnection(TcpConnection * connection);
    void RemoveConnection(TcpConnection * connection);

    virtual void OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);
    //创建连接函数
    virtual TcpConnection * NewConnection(TcpServer & server);
    //销毁连接函数
    virtual void DestroyConnection(TcpConnection * connection);
    //定时器处理函数
    virtual void OnTick();

protected:
    uint32_t _generate_id;

    uv_timer_t _tick_handle;
    ConnectionSet _connections;

    uint32_t _tick;
    uint32_t _max_out_buffer_size;

    char _host[256];
    uint16_t _port;
};

#endif