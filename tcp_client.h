#ifndef __LIB_COMMON_TCP_CLIENT_H__
#define __LIB_COMMON_TCP_CLIENT_H__

#include "tcp_service.h"

class TcpClient : public TcpService
{
    friend class TcpService;
    typedef TcpService Super;

public:
    TcpClient(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpClient();

    void Connect(const char * host, uint16_t port);

protected:
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
    uv_connect_t _tcp_connect_req;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;
    uint32_t _max_in_buffer_size;

    char _host[256];
    uint16_t _port;
};

#endif