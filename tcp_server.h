#ifndef __LIBCOMMON_TCP_SERVER_H__
#define __LIBCOMMON_TCP_SERVER_H__

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

    Socket * AllocateSocket();
    void ReleaseSocket(Socket * socket);
    void DestroySocket(Socket * socket);
    void ReleaseSockets();

    // 发送异步消息
    void AppendMessage(action_t action);

    // 新建连接事件
    virtual void OnConnectionCreated();
    // 建立连接事件
    virtual void OnConnectionEstablished(Socket * socket, Buffer * address) = 0;
    // 关闭连接事件
    virtual void OnConnectionClosed(Socket * socket);
    // 销毁连接事件
    virtual void OnConnectionDestroyed();

    static void ThreadReqCb(uv_async_t * handle);
    static void NewConnection(uv_stream_t * stream, int status);
    static void AfterClose(uv_handle_t * handle);
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);

private:
    char host_[256];        // 主机名
    uint16_t port_;         // 端口

    typedef TNodeList<Socket> SocketList;

    Logger * logger_;
    state_t state_;
    SocketList active_list_;
    SocketList free_list_;
    const size_t max_free_sockets_;
    LockQueue<action_t> outgoing_message_queue_;

    uv_loop_t loop_;
    uv_tcp_t tcp_;
    uv_async_t thread_req_;
    uv_sem_t thread_start_sem_;
};

class TcpServer::Socket : public BaseList::BaseNode, public SocketOpt, public NonCopyAble {
public:
    void AddRef();
    void Release();

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
    // 断开连接
    void Shutdown();
    // 关闭连接
    void Close();
    // 清理
    void Clear();

private:
    friend class TcpServer;
    explicit Socket(TcpServer & server);
    ~Socket();

private:
    TcpServer & server_;
    std::atomic_long ref_;
    Buffer * recv_buffer_;
    Buffer * send_buffer_;

    uv_tcp_t tcp_;
};

#endif