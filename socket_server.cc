#include "socket_server.h"
#include "exception.h"
#include "common.h"

enum IO_Operation
{
    IO_Read_Request,
    IO_Read_Completed,
    IO_Write_Request,
    IO_Write_Completed,
    IO_Close
};

typedef struct
{
    uv_write_t req;
    Buffer * buffer;
} write_req_t;

typedef struct
{
    uv_tcp_t socket;
    Buffer * buffer;
} read_tcp_t;

SocketServer::SocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size, ILog * logger)
    : BaseService(logger), Buffer::Allocator(buffer_size, max_free_buffers), _max_free_sockets(max_free_sockets), _is_listening(false)
{
    this->_loop->data = this;
    uv_async_init(this->_loop, &this->_accept_connections_event, SocketServer::AcceptConnectionsCb);

    memset(this->_host, 0, sizeof(this->_host));
    this->_port = 0;
}

SocketServer::~SocketServer()
{
    try
    {
        this->ReleaseSockets();
    }
    catch (...)
    {

    }
}

void SocketServer::Open(const char * host, uint16_t port)
{
    strncpy(this->_host, host, sizeof(this->_host));
    this->_port = port;

    this->Start();
}

void SocketServer::StartAcceptingConnections()
{
    if (!this->_is_listening)
    {
        /*
         * Call to unqualified virtual function
         */
        this->OnStartAcceptingConnections();

        uv_async_send(&this->_accept_connections_event);
    }
}

void SocketServer::StopAcceptingConnections()
{
    if (this->_is_listening)
    {
        uv_async_send(&this->_accept_connections_event);

        /*
         * Call to unqualified virtual function
         */
        this->OnStopAcceptingConnections();
    }
}

void SocketServer::InitiateShutdown()
{
    this->StopAcceptingConnections();

    /*
     * Call to unqualified virtual function
     */
    this->OnShutdownInitiated();
}

void SocketServer::WaitForShutdownToComplete()
{
    this->InitiateShutdown();

    this->Stop();
}

void SocketServer::ReleaseSockets()
{
    Mutex::Owner lock(this->_socket_lock);

    for (auto & it: this->_active_list)
    {
        it->Close();
    }

    while (!this->_active_list.empty())
    {
        this->ReleaseSocket(this->_active_list.front());
    }

    while (!this->_free_list.empty())
    {
        this->DestroySocket(this->_free_list.front());
        this->_free_list.pop_front();
    }

    if (this->_active_list.size() + this->_free_list.size() != 0 && this->_logger)
    {
        this->_logger->Error("SocketServer::ReleaseSockets() - Leaked sockets");
    }
}

void SocketServer::ReleaseBuffers()
{
    this->Flush();
}

void SocketServer::Run()
{
    try
    {
        BaseService::Run();
    }
    catch (const BaseException & ex)
    {
        if (this->_logger)
            this->_logger->Error("SocketServer::Run() - Exception: %s - %s", ex.Where(), ex.Message());
    }
    catch (...)
    {
        if (this->_logger)
            this->_logger->Error("SocketServer::Run() - Unexpected exception");
    }

    this->OnShutdownComplete();
}

void SocketServer::OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5)
{
    if (msg_id == IO_Write_Request)
    {
        Socket * socket = (Socket *)param1;
        Buffer * buffer = (Buffer *)param2;

        if (socket->_socket)
        {
            buffer->SetupWrite();

            write_req_t * req = (write_req_t *)jc_malloc(sizeof(write_req_t));
            req->buffer = buffer;
            uv_write((uv_write_t *)req, (uv_stream_t *)socket->_socket, buffer->GetUVBuffer(), 1, SocketServer::WriteCompletedCb);

            if (param3 == 1)
            {
                /*
                * final write, now shutdown send side of connection
                */
                socket->Shutdown();
            }
        }
        else
        {
            if (socket->_server._logger)
                socket->_server._logger->Error("SocketServer::OnRecvMsg() - IO_Write_Request - socket handle is null");

            socket->Release();
            buffer->Release();
        }
    }
    else if (msg_id == IO_Close)
    {
        Socket * socket = (Socket *)param1;

        socket->Close();
        
        socket->Release();
    }
}

SocketServer::Socket * SocketServer::AllocateSocket(uv_tcp_t * the_socket)
{
    Mutex::Owner lock(this->_socket_lock);

    Socket * socket = NULL;

    if (!this->_free_list.empty())
    {
        socket = this->_free_list.front();
        this->_free_list.pop_front();

        socket->Attach(the_socket);
        socket->AddRef();
    }
    else
    {
        socket = new Socket(*this, the_socket);

        /*
         * Call to unqualified virtual function
         */
        this->OnConnectionCreated();
    }

    this->_active_list.push_back(socket);

    socket->Read();
    uv_read_start((uv_stream_t *)the_socket, SocketServer::AllocBufferCb, SocketServer::ReadCompletedCb);

    return socket;
}

void SocketServer::ReleaseSocket(Socket * socket)
{
    if (!socket)
        throw BaseException("SocketServer::ReleaseSocket()", "socket is null");

    Mutex::Owner lock(this->_socket_lock);

    this->_active_list.remove(socket);

    if (this->_max_free_sockets == 0 || this->_free_list.size() < this->_max_free_sockets)
    {
        this->_free_list.push_back(socket);
    }
    else
    {
        this->DestroySocket(socket);
    }
}

void SocketServer::DestroySocket(Socket * socket)
{
    delete socket;

    /*
     * Call to unqualified virtual function
     */
    this->OnConnectionDestroyed();
}

void SocketServer::PostAbortiveClose(Socket * socket)
{
    socket->AddRef();

    AppMessage msg;
    msg.msg_id = IO_Close;
    msg.param1 = (uint64_t)socket;
    this->_msg_queue.Push(msg);
}

void SocketServer::Read(Socket * socket, Buffer * buffer)
{
    if (!buffer)
        buffer = this->Allocate();
    else
        buffer->AddRef();
        
    socket->AddRef();

    ((read_tcp_t *)socket->_socket)->buffer = buffer;
}

void SocketServer::Write(Socket * socket, const char * data, size_t data_length, bool then_shutdown)
{
    if (!socket || !data || data_length == 0) return;

    Buffer * buffer = this->Allocate();

    this->PreWrite(socket, buffer, data, data_length);

    buffer->AddData(data, data_length);

    socket->AddRef();

    AppMessage msg;
    msg.msg_id = IO_Write_Request;
    msg.param1 = (uint64_t)socket;
    msg.param2 = (uint64_t)buffer;
    msg.param3 = then_shutdown ? 1 : 0;
    this->_msg_queue.Push(msg);
}

void SocketServer::Write(Socket * socket, Buffer * buffer, bool then_shutdown)
{
    if (!socket || !buffer) return;

    buffer->AddRef();
    
    socket->AddRef();

    AppMessage msg;
    msg.msg_id = IO_Write_Request;
    msg.param1 = (uint64_t)socket;
    msg.param2 = (uint64_t)buffer;
    msg.param3 = then_shutdown ? 1 : 0;
    this->_msg_queue.Push(msg);
}

void SocketServer::AcceptConnectionsCb(uv_async_t * handle)
{
    SocketServer * service = (SocketServer *)handle->loop->data;

    if (service->_is_listening)
    {
        uv_close((uv_handle_t *)&service->_listening_socket, NULL);

        service->_is_listening = false;
    }
    else
    {
        union {
            struct sockaddr addr;
            struct sockaddr_in addr4;
            struct sockaddr_in6 addr6;
        } s;

        int r;
        r = uv_ip4_addr(service->_host, service->_port, &s.addr4);
        if (r != 0)
            r = uv_ip6_addr(service->_host, service->_port, &s.addr6);
        
        if (r == 0)
            r = uv_tcp_init(service->_loop, &service->_listening_socket);

        if (r == 0)
            r = uv_tcp_bind(&service->_listening_socket, &s.addr, 0);

        if (r == 0)
            r = uv_listen((uv_stream_t *)&service->_listening_socket, 128, SocketServer::OnAcceptCb);

        if (r != 0)
            throw BaseException("SocketServer::AcceptConnectionsCb()", uv_strerror(r));

        service->_is_listening = true;
    }
}

void SocketServer::OnAcceptCb(uv_stream_t * server, int status)
{
    SocketServer * service = (SocketServer *)server->loop->data;

    if (status < 0)
    {
        if (service->_logger)
            service->_logger->Error("SocketServer::OnAcceptCb() - %s", uv_strerror(status));
        
        return;
    }

    read_tcp_t * accepted_socket = (read_tcp_t *)jc_malloc(sizeof(read_tcp_t));
    uv_tcp_init(service->_loop, (uv_tcp_t *)accepted_socket);

    int r;
    if ((r = uv_accept(server, (uv_stream_t *)accepted_socket)) == 0)
    {
        Buffer * address = service->Allocate();

        int address_size = (int)address->GetSize();

        r = uv_tcp_getpeername((uv_tcp_t *)accepted_socket, (sockaddr *)address->GetBuffer(), &address_size);
        if (r == 0)
        {
            address->Use(address_size);

            Socket * socket = service->AllocateSocket((uv_tcp_t *)accepted_socket);

            /*
             * Call to unqualified virtual function
             */
            service->OnConnectionEstablished(socket, address);
        }
        else
        {
            uv_close((uv_handle_t *)accepted_socket, (uv_close_cb)jc_free);

            if (service->_logger)
                service->_logger->Error("SocketServer::OnAcceptCb() - uv_tcp_getpeername: %s", uv_strerror(r));
        }

        address->Release();
    }
    else
    {
        uv_close((uv_handle_t *)accepted_socket, (uv_close_cb)jc_free);

        if (service->_logger)
            service->_logger->Error("SocketServer::OnAcceptCb() - uv_accept: %s", uv_strerror(r));
    }
}

void SocketServer::AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{
    Socket * socket = (Socket *)handle->data;

    if (!socket) return;

    Buffer * buffer = ((read_tcp_t *)socket->_socket)->buffer;

    buffer->SetupRead();

    *buf = *(buffer->GetUVBuffer());
}

void SocketServer::ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
{
    Socket * socket = (Socket *)stream->data;

    if (!socket) return;

    bool close_socket = false;

    Buffer * buffer = ((read_tcp_t *)socket->_socket)->buffer;

    if (nread >= 0)
    {
        buffer->Use(nread);

        /*
         * Call to unqualified virtual function
         */
        socket->_server.ReadCompleted(socket, buffer);
    }
    else
    {
        if (socket->_server._logger)
        {
            if (nread == UV_EOF)
                socket->_server._logger->Info("SocketServer::ReadCompletedCb() - UV_EOF - client connection dropped");
            else
                socket->_server._logger->Error("SocketServer::ReadCompletedCb() - %s", uv_strerror((int)nread));
        }

        close_socket = true;
    }

    socket->Release();
    buffer->Release();

    if (close_socket)
    {
        socket->Close();
    }
}

void SocketServer::WriteCompletedCb(uv_write_t * req, int status)
{
    Socket * socket = (Socket *)req->handle->data;

    if (!socket) return;

    Buffer * buffer = ((write_req_t *)req)->buffer;

    if (status < 0)
    {
        if (socket->_server._logger)
            socket->_server._logger->Error("SocketServer::WriteCompletedCb() - %s", uv_strerror(status));
    }
    
    /*
     * Call to unqualified virtual function
     */
    socket->_server.WriteCompleted(socket, buffer);

    socket->Release();
    buffer->Release();
}

/*
 * SocketServer::Socket
 */

SocketServer::Socket::Socket(SocketServer & server, uv_tcp_t * socket)
    : _server(server), _ref(1)
{
    if (!socket)
        throw BaseException("SocketServer::Socket::Socket()", "socket is null");

    this->_socket = socket;
    this->_socket->data = this;
}

SocketServer::Socket::~Socket()
{

}

void SocketServer::Socket::Attach(uv_tcp_t * socket)
{
    if (this->_socket)
        throw BaseException("SocketServer::Socket::Attach()", "socket already attached");

    this->_socket = socket;
    this->_socket->data = this;
}

void SocketServer::Socket::Detatch()
{
    if (!this->_socket)
        throw BaseException("SocketServer::Socket::Detatch()", "socket is null");

    this->_socket->data = NULL;
    this->_socket = NULL;
}

void SocketServer::Socket::Read(Buffer * buffer)
{
    this->_server.Read(this, buffer);
}

void SocketServer::Socket::Write(const char * data, size_t data_length, bool then_shutdown)
{
    this->_server.Write(this, data, data_length, then_shutdown);
}

void SocketServer::Socket::Write(Buffer * buffer, bool then_shutdown)
{
    this->_server.Write(this, buffer, then_shutdown);
}

void SocketServer::Socket::AddRef()
{
    this->_lock.Lock();
    ++this->_ref;
    this->_lock.Unlock();
}

void SocketServer::Socket::Release()
{
    this->_lock.Lock();
    int ref = --this->_ref;
    this->_lock.Unlock();

    if (ref == 0)
        this->_server.ReleaseSocket(this);
}

void SocketServer::Socket::Shutdown()
{
    uv_shutdown_t * req = (uv_shutdown_t *)jc_malloc(sizeof(uv_shutdown_t));
    uv_shutdown(req, (uv_stream_t *)this->_socket, SocketServer::Socket::ShutdownCb);
}

void SocketServer::Socket::Close()
{
    Mutex::Owner lock(this->_server._socket_lock);

    if (this->_socket)
    {
        uv_close((uv_handle_t *)this->_socket, (uv_close_cb)jc_free);

        this->Detatch();

        this->_server.OnConnectionClosed(this);

        this->Release();
    }
}

void SocketServer::Socket::AbortiveClose()
{
    this->_server.PostAbortiveClose(this);
}

void SocketServer::Socket::ShutdownCb(uv_shutdown_t * req, int status)
{
    Socket * socket = (Socket *)req->handle->data;

    if (!socket) return;

    socket->Close();

    jc_free(req);
}