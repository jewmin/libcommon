#ifndef __LIB_COMMON_TCP_SERVICE_H__
#define __LIB_COMMON_TCP_SERVICE_H__

#include "service.h"

class TcpService : public BaseService
{
    typedef BaseService Super;

public:
    explicit TcpService(ILog * logger = NULL);
    virtual ~TcpService();

protected:
    //定时器处理函数，子类继承
    virtual void OnTick() = 0;
    //连接成功通知函数，子类继承
    virtual void OnConnected() = 0;
    //连接失败通知函数，子类继承
    virtual void OnConnectFailed() = 0;
    //断开连接通知函数，子类继承
    virtual void OnDisconnect() = 0;
    //已断开连接通知函数，子类继承
    virtual void OnDisconnected() = 0;
    //接收到数据，通知处理函数，子类继承
    virtual void OnRecv(const char * data, int nread) = 0;

    //libuv回调处理函数
    static void TimerCallback(uv_timer_t * handle);
    static void ConnectionCallback(uv_stream_t * server, int status);
    static void CloseCallback(uv_handle_t * handle);
    static void ConnectCallback(uv_connect_t * req, int status);
    static void AllocBufferCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);

protected:
    union {
        uv_handle_t handle;
        uv_stream_t stream;
        uv_tcp_t tcp;
    } _handle;
};

#endif