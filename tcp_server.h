#ifndef __LIBCOMMON_TCP_SERVER_H__
#define __LIBCOMMON_TCP_SERVER_H__

#include <tuple>
#include <atomic>

#include "uv.h"
#include "thread.h"
#include "buffer.h"
#include "logger.h"
#include "list.hpp"
#include "network.h"
#include "queue.hpp"
#include "non_copy_able.hpp"

class TcpServer : public SocketOpt, protected BaseThread, protected Buffer::Allocator {
public:
    class Socket;
    friend class Socket;

    int Listen(const char * host, uint16_t port);
    void WaitForShutdownToComplete();

protected:
    TcpServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = nullptr);
    virtual ~TcpServer();

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

    // 新建连接事件
    virtual void OnConnectionCreated();
    // 建立连接事件
    virtual void OnConnectionEstablished(Socket * socket, Buffer * address) = 0;
    // 关闭连接事件
    virtual void OnConnectionClosed(Socket * socket);
    // 销毁连接事件
    virtual void OnConnectionDestroyed();
    // 读取完成事件，由派生类来重置读取缓冲区
    virtual void OnReadComplete(Socket * socket, Buffer * buffer) = 0;
    // 发送完成事件，返回已发送的数据长度，<0为错误码
    virtual void OnWriteComplete(Socket * socket, int written) = 0;

private:
    Socket * AllocateSocket();
    void ReleaseSocket(Socket * socket);
    void DestroySocket(Socket * socket);
    void ReleaseSockets();

    // 发送异步消息
    void AppendMessage(action_t action, Socket * socket);

    static void ThreadReqCb(uv_async_t * handle);
    static void NewConnection(uv_stream_t * stream, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void AfterWrite(uv_write_t * req, int status);

private:
    char host_[256];        // 主机名
    uint16_t port_;         // 端口

    typedef TNodeList<Socket> SocketList;

    Logger * logger_;
    state_t state_;
    SocketList active_list_;
    SocketList free_list_;
    const size_t max_free_sockets_;
    LockQueue<std::tuple<action_t, Socket *>> outgoing_message_queue_;

    uv_loop_t loop_;
    uv_tcp_t tcp_;
    uv_async_t thread_req_;
    uv_sem_t thread_start_sem_;
};

class TcpServer::Socket : public BaseList::BaseNode, public SocketOpt, public NonCopyAble {
public:
    uv_tcp_t * uv_tcp() {
        return &tcp_;
    }
    uv_stream_t * uv_stream() {
        return reinterpret_cast<uv_stream_t *>(&tcp_);
    }
    uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&tcp_);
    }

protected:
    // 添加引用
    void AddRef();
    // 释放引用
    void Release();
    // 发送数据到缓冲区
    bool SendData(const char * data, size_t length);
    // 通知关闭连接
    void AbortiveClose();

private:
    friend class TcpServer;
    explicit Socket(TcpServer & server);
    ~Socket();

    // 断开连接
    void Shutdown();
    // 关闭连接
    void Close();
    // 清理
    void Clear();
    // 提交发送
    void Send(const char * data, size_t length);

private:
    TcpServer & server_;
    std::atomic_long ref_;
    Buffer * recv_buffer_;
    Buffer * send_buffer_;
    Mutex send_buffer_lock_;
    size_t send_buffer_len_;

    uv_tcp_t tcp_;
    uv_write_t write_req_;
};

#endif