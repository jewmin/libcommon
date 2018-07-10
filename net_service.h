#ifndef __LIB_COMMON_NET_SERVICE_H__
#define __LIB_COMMON_NET_SERVICE_H__

#include "uv.h"
#include "thread.h"
#include "log.h"
#include "mutex.h"
#include "queue.h"

class NetService : public BaseThread
{
    //异步操作，通知类型
    enum NotifyType
    {
        NOTIFY_STOP_LOOP = 1, //终止事件循环
        NOTIFY_CREATE_SERVER = 2, //创建监听服务
        NOTIFY_CREATE_CLIENT = 3, //创建连接
        NOTIFY_CONNECTION_WRITE = 4, //发送数据
    };

    //异步操作
    struct NotifyOperation
	{
		NotifyType type;
	};

public:
    explicit NetService(ILog * logger = nullptr);
    virtual ~NetService();
    virtual void Stop();

protected:
    virtual void Run();
    virtual void OnTerminated();

    static void OnNotify(uv_async_t * handle);
    static void OnAccept(uv_connect_t * req, int status);
    static void OnConnect(uv_connect_t * req, int status);
    static void OnAlloc(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void OnRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buf);
    static void OnWrite(uv_write_t * req, int status);
    static void OnTimer(uv_timer_t * handle);
    static void OnShutDown(uv_shutdown_t * req, int status);
    static void OnClose(uv_handle_t * handle);

private:
    uv_loop_t * _loop;
    uv_async_t _async;
    ILog * _logger;
    Mutex _mutex;
    Queue<NotifyOperation> _operation_queue;
};

#endif