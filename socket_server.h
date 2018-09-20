#ifndef __LIB_COMMON_SOCKET_SERVER_H__
#define __LIB_COMMON_SOCKET_SERVER_H__

#include "uv.h"
#include "mutex.h"
#include "buffer.h"
#include "service.h"
#include "network.h"
#include "app_msg.h"
#include "container.h"
#include "queue.hpp"

class SocketServer : public SocketOpt, protected BaseService, private Buffer::Allocator
{
public:
    class Socket;
    friend class Socket;

    virtual ~SocketServer();

    int Open(const char * host, uint16_t port);

    void StartAcceptingConnections();
    void StopAcceptingConnections();

    void InitiateShutdown();
    void WaitForShutdownToComplete();

protected:
    SocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, Logger * logger = NULL);

    void Close();
    void Listen();

    void ReleaseSockets();
    void ReleaseBuffers();

    virtual void Run();

private:
    virtual void OnStartAcceptingConnections() {}
    virtual void OnStopAcceptingConnections() {}
    virtual void OnShutdownInitiated() {}
    virtual void OnShutdownComplete() {}

    virtual void OnListen() {}
    virtual void OnListenFail() {}
    virtual void OnClose() {}

    virtual void OnConnectionCreated() {}

    virtual void OnConnectionEstablished(Socket * socket, Buffer * address) = 0;

    virtual void OnConnectionClosed(Socket * socket) {}

    virtual void OnConnectionDestroyed() {}

    virtual void OnBufferCreated() {}
    virtual void OnBufferAllocated() {}
    virtual void OnBufferReleased() {}
    virtual void OnBufferDestroyed() {}

    Socket * AllocateSocket(uv_tcp_t * the_socket);
    void ReleaseSocket(Socket * socket);
    void DestroySocket(Socket * socket);

    void PostAbortiveClose(Socket * socket);

    void Read(Socket * socket, Buffer * buffer);
    void Write(Socket * socket, const char * data, size_t data_length, bool then_shutdown);
    void Write(Socket * socket, Buffer * buffer, bool then_shutdown);

    virtual void PreWrite(Socket * socket, Buffer * buffer, const char * data, size_t data_length) {}

    virtual void ReadCompleted(Socket * socket, Buffer * buffer) = 0;
    virtual void WriteCompleted(Socket * socket, Buffer * buffer, int status) = 0;

    /*
     * No copies do not implement
     */
    SocketServer(const SocketServer & rhs);
    SocketServer & operator =(const SocketServer & rhs);

private:
    static void AcceptConnectionsCb(uv_async_t * handle);
    static void OnCloseCb(uv_handle_t * handle);
    static void OnAcceptCb(uv_stream_t * server, int status);
    static void OnConnectionCloseCb(uv_handle_t * handle);
    static void AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void WriteCompletedCb(uv_write_t * req, int status);
    static void ConnectionsCb(uv_prepare_t * handle);

private:
    uv_tcp_t _listening_socket;
    uv_async_t _accept_connections_event;
    uv_async_t _connections_notify_event;
    uv_prepare_t _connections_event;
    
    Mutex _socket_lock;

    typedef std::list<Socket *> SocketList;
    typedef std::list<Socket *>::iterator SocketListIterator;

    SocketList _active_list;
    SocketList _free_list;
    LockQueue<AppMessage> _req_list;

    char _host[256];
    uint16_t _port;

    const size_t _max_free_sockets;
};

class SocketServer::Socket : public SocketOpt
{
public:
    friend class SocketServer;

    void Read(Buffer * buffer = NULL);
    void Write(const char * data, size_t data_length, bool then_shutdown = false);
    void Write(Buffer * buffer, bool then_shutdown = false);
    void Shutdown();

    void AddRef();
    void Release();

    void AbortiveClose();

protected:
    void TryWrite();
    void Close();

private:
    Socket(SocketServer & server, uv_tcp_t * socket);
    ~Socket();

    void Attach(uv_tcp_t * socket);
    void Detatch();
    void SetupRead(Buffer * buffer);

    /*
     * No copies do not implement
     */
    Socket(const Socket & rhs);
    Socket & operator =(const Socket & rhs);

private:
    SocketServer & _server;
    uv_tcp_t * _socket;
    DoubleBuffer<Buffer *> _write_buffers;

    int _ref;
    Mutex _lock;
};

#endif