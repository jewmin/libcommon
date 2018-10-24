#ifndef __LIBCOMMON_TCP_SOCKET_H__
#define __LIBCOMMON_TCP_SOCKET_H__

#include "uv.h"
#include "network.h"

class EventLoop;
class TcpSocket : public SocketOpt {
protected:
    TcpSocket(EventLoop * loop);
    virtual ~TcpSocket();

    uv_tcp_t * uv_tcp() {
        return &tcp_;
    }
    uv_stream_t * uv_stream() {
        return reinterpret_cast<uv_stream_t *>(&tcp_);
    }
    uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&tcp_);
    }
    uv_loop_t * uv_loop() {
        return nullptr;
    }

    int Connect();
    static void AfterConnect(uv_connect_t * req, int status);
    virtual void OnConnected();
    virtual void OnConnectFailed();

    void Shutdown();
    void Close();
    static void AfterClose(uv_handle_t * handle);
    virtual void OnDisconnected();

    void ReadStart();
    void ReadStop();
    static void AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    virtual void OnReadComplete() = 0;

private:
    char name_[64];
    char host_[256];
    uint16_t port_;

    uv_tcp_t tcp_;
    EventLoop * loop_;
};

#endif