#ifndef __LIB_COMMON_TCP_SERVER_H__
#define __LIB_COMMON_TCP_SERVER_H__

#include "tcp_service.h"

class TcpConnection;
class TcpServer : public TcpService
{
    friend class TcpService;
    friend class TcpConnection;
    typedef TcpService Super;

public:
    TcpServer(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpServer();

    void Listen(const char * host, uint16_t port);

protected:
    //创建连接函数，子类继承
    virtual TcpConnection * NewConnection(TcpServer & server) = 0;
    //销毁连接函数，子类继承
    virtual void DestroyConnection(TcpConnection * connection) = 0;
    //已关闭连接函数，子类继承
    virtual void OnClosed();
    //定时器处理函数，子类继承
    virtual void OnTick();
    //连接成功通知函数，子类继承
    virtual void OnConnected();
    //连接失败通知函数，子类继承
    virtual void OnConnectFailed();
    //断开连接通知函数，子类继承
    virtual void OnDisconnect();
    //已断开连接通知函数，子类继承
    virtual void OnDisconnected();
    //接收到数据，通知处理函数，子类继承
    virtual void OnRecv(const char * data, int nread);

protected:
    uv_timer_t _tick_handle;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;
    uint32_t _max_in_buffer_size;

    char _host[256];
    uint16_t _port;
};

#endif