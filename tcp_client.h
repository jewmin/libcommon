#ifndef __LIBCOMMON_TCP_CLIENT_H__
#define __LIBCOMMON_TCP_CLIENT_H__

#include <time.h>

#include "uv.h"
#include "mutex.h"
#include "thread.h"
#include "logger.h"
#include "buffer.h"
#include "network.h"

class TcpClient : public SocketOpt, protected BaseThread, protected Buffer::Allocator
{
    typedef enum {
        kNew,
        kRunning,
        kDone
    } state_t;

public:
    int ConnectTo(const char * host, uint16_t port);
    void WaitForShutdownToComplete();

protected:
    TcpClient(size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr);
    virtual ~TcpClient();

    uv_tcp_t * tcp() {
        return &tcp_;
    }
    uv_stream_t * stream() {
        return reinterpret_cast<uv_stream_t *>(&tcp_);
    }
    uv_handle_t * handle() {
        return reinterpret_cast<uv_handle_t *>(&tcp_);
    }

    void Run() override;
    void OnTerminated() override;

    int ConnectToServer();
    void Shutdown();
    void Close();

    virtual void OnConnected();
    virtual void OnConnectFailed();
    virtual void OnDisconnected();
    virtual void OnRead(Buffer * buffer) = 0;

    static void ThreadReqCb(uv_async_t * handle);
    static void AfterConnect(uv_connect_t * req, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);

private:
    char host_[256];        // 主机名
    uint16_t port_;         // 端口

    Logger * logger_;
    state_t state_;
    Buffer * recv_buffer_;
    Buffer * send_buffer_;
    Mutex send_buffer_lock_;

    uv_loop_t loop_;
    uv_tcp_t tcp_;
    uv_connect_t connect_;
    uv_async_t thread_req_;
    uv_sem_t thread_start_sem_;
};

#endif