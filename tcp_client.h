#ifndef __LIBCOMMON_TCP_CLIENT_H__
#define __LIBCOMMON_TCP_CLIENT_H__

#include "uv.h"
#include "mutex.h"
#include "thread.h"
#include "logger.h"
#include "buffer.h"
#include "network.h"
#include "queue.hpp"

class TcpClient : public SocketOpt, protected BaseThread, protected Buffer::Allocator {
public:
    int ConnectTo(const char * host, uint16_t port);
    void WaitForShutdownToComplete();

protected:
    TcpClient(size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr);
    virtual ~TcpClient();

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
    // 发送数据到缓冲区
    bool SendData(const char * data, size_t length);
    // 提交发送
    void Send(const char * data, size_t length);
    // 发送异步消息
    void AppendMessage(action_t action);

    // 连接成功事件
    virtual void OnConnected();
    // 连接失败事件
    virtual void OnConnectFailed();
    // 断开连接事件
    virtual void OnDisconnected();
    // 读取完成事件，由派生类来重置读取缓冲区
    virtual void OnReadComplete(Buffer * buffer) = 0;
    // 发送完成事件，返回已发送的数据长度，<0为错误码
    virtual void OnWriteComplete(int written) = 0;

    static void ThreadReqCb(uv_async_t * handle);
    static void AfterConnect(uv_connect_t * req, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void AfterWrite(uv_write_t * req, int status);

private:
    char host_[256];        // 主机名
    uint16_t port_;         // 端口

    Logger * logger_;
    state_t state_;
    Buffer * recv_buffer_;
    Buffer * send_buffer_;
    Mutex send_buffer_lock_;
    size_t send_buffer_len_;
    LockQueue<action_t> outgoing_message_queue_;

    uv_loop_t loop_;
    uv_tcp_t tcp_;
    uv_connect_t connect_req_;
    uv_write_t write_req_;
    uv_async_t thread_req_;
    uv_sem_t thread_start_sem_;
};

#endif