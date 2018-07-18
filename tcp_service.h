#ifndef __LIB_COMMON_TCP_SERVICE_H__
#define __LIB_COMMON_TCP_SERVICE_H__

#include "service.h"

class TcpService : public BaseService
{
    typedef BaseService Super;

public:
    explicit TcpService(uint32_t max_in_buffer_size, ILog * logger = NULL);
    virtual ~TcpService();

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

    uint32_t _max_in_buffer_size;
};

#endif