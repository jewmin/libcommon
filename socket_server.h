#ifndef __LIB_COMMON_SOCKET_SERVER_H__
#define __LIB_COMMON_SOCKET_SERVER_H__

#include "uv.h"
#include <list>
#include "mutex.h"
#include "buffer.h"
#include "service.h"

class SocketServer : protected BaseService, private Buffer::Allocator
{
public:
    class Socket;
    friend class Socket;

    virtual ~SocketServer();

    void Open(const char * host, uint16_t port);

    void StartAcceptingConnections();
    void StopAcceptingConnections();

    void InitiateShutdown();
    void WaitForShutdownToComplete();

protected:
    SocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, ILog * logger = NULL);

    void ReleaseSockets();
    void ReleaseBuffers();

    virtual void Run();
    virtual void OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5);

private:
    virtual void OnStartAcceptingConnections() {}
    virtual void OnStopAcceptingConnections() {}
    virtual void OnShutdownInitiated() {}
    virtual void OnShutdownComplete() {}

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
    virtual void WriteCompleted(Socket * socket, Buffer * buffer) = 0;

    /*
     * No copies do not implement
     */
    SocketServer(const SocketServer & rhs);
    SocketServer & operator =(const SocketServer & rhs);

private:
    static void AcceptConnectionsCb(uv_async_t * handle);
    static void OnAcceptCb(uv_stream_t * server, int status);
    static void AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
    static void ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
    static void WriteCompletedCb(uv_write_t * req, int status);

private:
    uv_tcp_t _listening_socket;
    uv_async_t _accept_connections_event;
    
    Mutex _socket_lock;

    typedef std::list<Socket *> SocketList;
    typedef std::list<Socket *>::iterator SocketListIterator;

    SocketList _active_list;
    SocketList _free_list;

    bool _is_listening;

    char _host[256];
    uint16_t _port;

    const size_t _max_free_sockets;
};

class SocketServer::Socket
{
public:
    friend class SocketServer;

    void Read(Buffer * buffer = NULL);
    void Write(const char * data, size_t data_length, bool then_shutdown = false);
    void Write(Buffer * buffer, bool then_shutdown = false);

    void AddRef();
    void Release();

    void Shutdown();
    void Close();
    void AbortiveClose();

private:
    Socket(SocketServer & server, uv_tcp_t * socket);
    ~Socket();

    void Attach(uv_tcp_t * socket);
    void Detatch();

    /*
     * No copies do not implement
     */
    Socket(const Socket & rhs);
    Socket & operator =(const Socket & rhs);

private:
    static void ShutdownCb(uv_shutdown_t * req, int status);

private:
    SocketServer & _server;
    uv_tcp_t * _socket;

    int _ref;
    Mutex _lock;
};

#endif