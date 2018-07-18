#ifndef __LIB_COMMON_TCP_CLIENT_H__
#define __LIB_COMMON_TCP_CLIENT_H__

#include "tcp_service.h"

class TcpClient : public TcpService
{
    friend class TcpService;
    typedef TcpService Super;

public:
    TcpClient(const char * name, uint32_t tick, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size, uint32_t reconnect_tick, ILog * logger = NULL);
    virtual ~TcpClient();

    int Connect(const char * host, uint16_t port);
    int ReConnect();

protected:
    //正在关闭事件通知函数
    virtual void OnClosing();
    //已关闭事件通知函数
    virtual void OnClosed();
    //定时器处理函数
    virtual void OnTick();
    //连接成功事件通知函数
    virtual void OnConnected();
    //连接失败事件通知函数
    virtual void OnConnectFailed();
    //断开连接事件通知函数
    virtual void OnDisconnect();
    //已断开连接事件通知函数
    virtual void OnDisconnected();
    //数据接收处理事件通知函数
    virtual void OnRecv(const char * data, int nread);

    //libuv回调处理函数
    static void ReConnectTimerCallback(uv_timer_t * handle);

protected:
    uv_timer_t _tick_handle;
    uv_timer_t _reconnect_tick_handle;
    uv_connect_t _tcp_connect_req;

    char _name[64];
    uint32_t _tick;
    uint32_t _max_out_buffer_size;
    uint32_t _reconnect_tick;

    char _host[256];
    uint16_t _port;
};

#endif