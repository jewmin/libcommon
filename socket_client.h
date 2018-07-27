#ifndef __LIB_COMMON_SOCKET_CLIENT_H__
#define __LIB_COMMON_SOCKET_CLIENT_H__

#include "uv.h"
#include "service.h"
#include "buffer.h"

class SocketClient : protected BaseService, private Buffer::Allocator
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

    void ReleaseBuffers();

    virtual void Run();
    virtual void OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

private:
    virtual void OnStartConnections() {}
    virtual void OnStopConnections() {}
    virtual void OnShutdownInitiated() {}
    virtual void OnShutdownComplete() {}

    virtual void OnConnect() {}
    virtual void OnConnectFail() {}
    virtual void OnClose() {}

    void Read(Buffer * buffer = NULL);
    virtual void ReadCompleted(Buffer * buffer) = 0;
    virtual void WriteCompleted(Buffer * buffer) = 0;

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

private:
    uv_tcp_t _connect_socket;
    uv_connect_t _req;
    uv_async_t _connect_event;

    bool _is_connected;
    bool _is_connecting;

    char _host[256];
    uint16_t _port;
};

#endif