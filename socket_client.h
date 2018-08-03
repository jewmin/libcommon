#ifndef __LIB_COMMON_SOCKET_CLIENT_H__
#define __LIB_COMMON_SOCKET_CLIENT_H__

#include "uv.h"
#include "buffer.h"
#include "service.h"
#include "network.h"
#include "double_buffer.h"

class SocketClient : protected BaseService, private Buffer::Allocator, public SocketOpt
{
public:
    virtual ~SocketClient();

    void ConnectTo(const char * host, uint16_t port);

    void StartConnections();
    void StopConnections();

    void InitiateShutdown();
    void WaitForShutdownToComplete();

    void Write(const char * data, size_t data_length);

protected:
    SocketClient(size_t max_free_buffers, size_t buffer_size = 1024, ILog * logger = NULL);

    virtual void PreWrite(Buffer * buffer, const char * data, size_t data_length) {}
    
    void Read(Buffer * buffer = NULL);

    void TryWrite();

    void Shutdown();

    void Close();

    void Connect();

    void ReleaseBuffers();

    virtual void Run();

private:
    virtual void OnStartConnections() {}
    virtual void OnStopConnections() {}
    virtual void OnShutdownInitiated() {}
    virtual void OnShutdownComplete() {}

    virtual void OnConnect() {}
    virtual void OnConnectFail() {}
    virtual void OnClose() {}

    virtual void ReadCompleted(Buffer * buffer) = 0;
    virtual void WriteCompleted(Buffer * buffer, int status) = 0;

    /*
     * No copies do not implement
     */
    SocketClient(const SocketClient & rhs);
    SocketClient & operator =(const SocketClient & rhs);

private:
    static void ConnectionsCb(uv_async_t * handle);
    static void OnConnectCb(uv_connect_t * req, int status);
    static void OnCloseCb(uv_handle_t * handle);
    static void AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void WriteCompletedCb(uv_write_t * req, int status);
    static void WriteIdleCb(uv_idle_t * handle);

private:
    uv_tcp_t _connect_socket;
    uv_connect_t _req;
    uv_async_t _connect_event;
    uv_idle_t _write_idle_event;
    DoubleBuffer<Buffer *> _write_buffers;

    char _host[256];
    uint16_t _port;
};

#endif