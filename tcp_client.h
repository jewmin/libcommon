#ifndef __LIBCOMMON_TCP_CLIENT_H__
#define __LIBCOMMON_TCP_CLIENT_H__

#include <vector>

#include "uv.h"
#include "mutex.h"
#include "thread.h"
#include "logger.h"
#include "buffer.h"
#include "network.h"

class TcpClient : public SocketOpt, protected BaseThread, protected Buffer::Allocator {
public:
    int ConnectTo(const char * host, uint16_t port);
    void InitiateShutdown();
    void WaitForShutdownToComplete();
    void RunInLoop(const Functor & cb);
    void QueueInLoop(const Functor & cb);

protected:
    TcpClient(const char * name, size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr);
    virtual ~TcpClient();

    const char * name() {
        return name_;
    }
    Logger * logger() {
        return logger_;
    }
    uv_tcp_t * uv_tcp() {
        return &tcp_;
    }
    uv_stream_t * uv_stream() {
        return reinterpret_cast<uv_stream_t *>(&tcp_);
    }
    uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&tcp_);
    }

    void Run() override;
    void OnTerminated() override;

    // 连接
    int ConnectToServer();
    // 断开连接
    void Shutdown();
    // 关闭连接
    void Close();
    // 发送，只能在循环中调用
    int SendInLoop(const char * data, size_t length, const WriteCallback & cb = nullptr);

    // 连接成功事件
    virtual void OnConnected();
    // 连接失败事件
    virtual void OnConnectFailed();
    // 断开连接事件
    virtual void OnDisconnected();
    // 读取完成事件
    virtual void OnReadComplete(Buffer * buffer) = 0;

private:
    static void ThreadReqCb(uv_async_t * handle);
    static void AfterConnect(uv_connect_t * req, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void AfterWrite(uv_write_t * req, int status);

private:
    char name_[64];         // 名称
    char host_[256];        // 主机名
    uint16_t port_;         // 端口

    Logger * logger_;
    state_t state_;
    Buffer * recv_buffer_;
    Mutex functor_lock_;
    std::vector<Functor> pending_functors_;

    uv_loop_t loop_;
    uv_tcp_t tcp_;
    uv_connect_t connect_req_;
    uv_async_t thread_req_;
    uv_sem_t thread_start_sem_;
};

#endif