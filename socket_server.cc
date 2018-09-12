#include "socket_server.h"
#include "exception.h"

enum Request_Type
{
    IO_Write_Request,
    IO_Close
};

typedef struct
{
    uv_tcp_t handle;
    SocketServer::Socket * socket;
    Buffer * buffer;
} connection_t;

SocketServer::SocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size, ILog * logger)
    : BaseService(logger), Buffer::Allocator(buffer_size, max_free_buffers), _max_free_sockets(max_free_sockets)
{
    this->_loop->data = this;
    uv_async_init(this->_loop, &this->_accept_connections_event, SocketServer::AcceptConnectionsCb);
    uv_async_init(this->_loop, &this->_connections_notify_event, NULL);
    uv_prepare_init(this->_loop, &this->_connections_event);

    memset(this->_host, 0, sizeof(this->_host));
    this->_port = 0;
}

SocketServer::~SocketServer()
{
    
}

int SocketServer::Open(const char * host, uint16_t port)
{
    strncpy(this->_host, host, sizeof(this->_host));
    this->_port = port;

    return this->Start();
}

void SocketServer::StartAcceptingConnections()
{
    if (this->GetStatus() == SocketOpt::S_DISCONNECTED)
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
    if (this->GetStatus() == SocketOpt::S_CONNECTED)
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

void SocketServer::Close()
{
    if (this->GetStatus() != SocketOpt::S_CONNECTED) return;

    this->SetStatus(SocketOpt::S_DISCONNECTING);

    if (this->HasFlag(SocketOpt::F_CLOSING)) return;

    this->AddFlag(SocketOpt::F_CLOSING);

    uv_close((uv_handle_t *)&this->_listening_socket, SocketServer::OnCloseCb);
}

void SocketServer::Listen()
{
    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;

    int r;
    r = uv_ip4_addr(this->_host, this->_port, &s.addr4);
    if (r != 0)
        r = uv_ip6_addr(this->_host, this->_port, &s.addr6);

    if (r == 0)
        r = uv_tcp_init(this->_loop, &this->_listening_socket);

    if (r == 0)
        r = uv_tcp_bind(&this->_listening_socket, &s.addr, 0);

    if (r == 0)
        r = uv_listen((uv_stream_t *)&this->_listening_socket, 128, SocketServer::OnAcceptCb);

    this->ClearFlag();

    if (r == 0)
    {
        this->SetStatus(SocketOpt::S_CONNECTED);

        this->AddFlag(SocketOpt::F_LISTEN);

        /*
        * Call to unqualified virtual function
        */
        this->OnListen();
    }
    else
    {
        if (this->_logger)
            this->_logger->Error("SocketServer::AcceptConnectionsCb() - %s", uv_strerror(r));

        /*
        * Call to unqualified virtual function
        */
        this->OnListenFail();
    }
}

void SocketServer::ReleaseSockets()
{
    this->_socket_lock.Lock();

    for (auto & it : this->_active_list)
    {
        it->Shutdown();
    }
    
    this->_socket_lock.Unlock();

    while (!this->_active_list.empty())
    {
        uv_run(this->_loop, UV_RUN_ONCE);
    }

    Mutex::Guard lock(this->_socket_lock);

    while (!this->_free_list.empty())
    {
        this->DestroySocket(this->_free_list.front());
        this->_free_list.pop_front();
    }
}

void SocketServer::ReleaseBuffers()
{
    this->Flush();
}

void SocketServer::Run()
{
    uv_prepare_start(&this->_connections_event, SocketServer::ConnectionsCb);
    uv_prepare_start(&this->_msg_handle, BaseService::MsgCallback);
    uv_run(this->_loop, UV_RUN_DEFAULT);
    while (this->GetStatus() != SocketOpt::S_DISCONNECTED) uv_run(this->_loop, UV_RUN_ONCE);
    this->ReleaseSockets();
    this->ReleaseBuffers();

    /*
    * Call to unqualified virtual function
    */
    this->OnShutdownComplete();
}

SocketServer::Socket * SocketServer::AllocateSocket(uv_tcp_t * the_socket)
{
    Mutex::Guard lock(this->_socket_lock);

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

    socket->SetStatus(SocketOpt::S_CONNECTED);

    socket->AddFlag(SocketOpt::F_ACCEPT);

    socket->Read();

    uv_read_start((uv_stream_t *)the_socket, SocketServer::AllocBufferCb, SocketServer::ReadCompletedCb);

    return socket;
}

void SocketServer::ReleaseSocket(Socket * socket)
{
    if (!socket)
        throw BaseException("SocketServer::ReleaseSocket()", "socket is null");

    Mutex::Guard lock(this->_socket_lock);

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
    if (socket->GetStatus() != SocketOpt::S_CONNECTED) return;

    socket->AddRef();

    AppMessage req;
    req.msg_id = IO_Close;
    req.param1 = (uint64_t)socket;
    this->_req_list.Push(req);
    uv_async_send(&this->_connections_notify_event);
}

void SocketServer::Read(Socket * socket, Buffer * buffer)
{
    if (socket->HasFlag(SocketOpt::F_READING)) return;

    if (!buffer)
        buffer = this->Allocate();
    else
        buffer->AddRef();
        
    socket->AddRef();

    socket->SetupRead(buffer);
}

void SocketServer::Write(Socket * socket, const char * data, size_t data_length, bool then_shutdown)
{
    if (!socket || !data || data_length == 0) return;

    if (socket->GetStatus() != SocketOpt::S_CONNECTED) return;

    Buffer * buffer = this->Allocate();

    buffer->Use(sizeof(uv_write_t));

    this->PreWrite(socket, buffer, data, data_length);

    if (buffer->GetSize() < buffer->GetUsed() || buffer->GetSize() - buffer->GetUsed() < data_length)
    {
        if (this->_logger)
            this->_logger->Error("SocketServer::Write() - %s", uv_strerror(UV_ENOBUFS));

        buffer->Release();
        return;
    }

    buffer->AddData(data, data_length);

    socket->AddRef();

    AppMessage req;
    req.msg_id = IO_Write_Request;
    req.param1 = (uint64_t)socket;
    req.param2 = (uint64_t)buffer;
    req.param3 = then_shutdown ? 1 : 0;
    this->_req_list.Push(req);
    uv_async_send(&this->_connections_notify_event);
}

void SocketServer::Write(Socket * socket, Buffer * buffer, bool then_shutdown)
{
    if (!socket || !buffer) return;

    if (socket->GetStatus() != SocketOpt::S_CONNECTED) return;

    buffer->AddRef();
    
    socket->AddRef();

    AppMessage req;
    req.msg_id = IO_Write_Request;
    req.param1 = (uint64_t)socket;
    req.param2 = (uint64_t)buffer;
    req.param3 = then_shutdown ? 1 : 0;
    this->_req_list.Push(req);
    uv_async_send(&this->_connections_notify_event);
}

void SocketServer::AcceptConnectionsCb(uv_async_t * handle)
{
    SocketServer * service = (SocketServer *)handle->loop->data;

    if (service->GetStatus() == SocketOpt::S_CONNECTED)
    {
        service->Close();
    }
    else
    {
        service->Listen();
    }
}

void SocketServer::OnCloseCb(uv_handle_t * handle)
{
    SocketServer * service = (SocketServer *)handle->loop->data;

    service->SetStatus(SocketOpt::S_DISCONNECTED);

    /*
    * Call to unqualified virtual function
    */
    service->OnClose();
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
    else if (service->GetStatus() != SocketOpt::S_CONNECTED)
    {
        if (service->_logger)
            service->_logger->Error("SocketServer::OnAcceptCb() - service is not connected");

        return;
    }

    connection_t * accepted_socket = (connection_t *)jc_malloc(sizeof(connection_t));
    uv_tcp_init(service->_loop, (uv_tcp_t *)accepted_socket);

    int r;
    r = uv_accept(server, (uv_stream_t *)accepted_socket);
    
    if (r == 0)
        r = uv_tcp_nodelay((uv_tcp_t *)accepted_socket, service->GetNoDelay() ? 1 : 0);

    if (r == 0)
    {
        if (service->GetKeepAlive() > 0)
            r = uv_tcp_keepalive((uv_tcp_t *)accepted_socket, 1, service->GetKeepAlive());
        else
            r = uv_tcp_keepalive((uv_tcp_t *)accepted_socket, 0, 0);
    }

    if (r == 0)
    {
        Buffer * address = service->Allocate();

        int address_size = (int)address->GetSize();

        r = uv_tcp_getpeername((uv_tcp_t *)accepted_socket, (sockaddr *)address->GetBuffer(), &address_size);
        if (r == 0)
        {
            if (address_size > address->GetSize())
            {
                r = UV_EFAULT;
            }
            else
            {
                address->Use(address_size);

                Socket * socket = service->AllocateSocket((uv_tcp_t *)accepted_socket);

                /*
                * Call to unqualified virtual function
                */
                service->OnConnectionEstablished(socket, address);
            }
        }

        address->Release();
    }

    if (r != 0)
    {
        uv_close((uv_handle_t *)accepted_socket, (uv_close_cb)jc_free);

        if (service->_logger)
            service->_logger->Error("SocketServer::OnAcceptCb() - %s", uv_strerror(r));
    }
}

void SocketServer::OnConnectionCloseCb(uv_handle_t * handle)
{
    connection_t * connection = (connection_t *)handle;

    Socket * socket = connection->socket;

    socket->SetStatus(SocketOpt::S_DISCONNECTED);

    /*
    * Call to unqualified virtual function
    */
    socket->_server.OnConnectionClosed(socket);

    if (socket->HasFlag(SocketOpt::F_READING))
    {
        socket->RemoveFlag(SocketOpt::F_READING);

        connection->buffer->Release();
        socket->Release();
    }

    socket->Detatch();

    socket->Release();

    jc_free(handle);
}

void SocketServer::AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{
    connection_t * connection = (connection_t *)handle;

    Socket * socket = connection->socket;

    if (socket->GetStatus() != SocketOpt::S_CONNECTED || !socket->HasFlag(SocketOpt::F_READING)) return;

    Buffer * buffer = connection->buffer;

    buffer->SetupRead();

    *buf = *(buffer->GetUVBuffer());
}

void SocketServer::ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
{
    connection_t * connection = (connection_t *)stream;

    Socket * socket = connection->socket;

    if (socket->GetStatus() != SocketOpt::S_CONNECTED || !socket->HasFlag(SocketOpt::F_READING))
    {
        uv_read_stop(stream);
        return;
    }

    socket->RemoveFlag(SocketOpt::F_READING);

    Buffer * buffer = connection->buffer;

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
            socket->_server._logger->Error("SocketServer::ReadCompletedCb() - %s", uv_strerror((int)nread));

        socket->Shutdown();
    }

    buffer->Release();
    socket->Release();
}

void SocketServer::WriteCompletedCb(uv_write_t * req, int status)
{
    connection_t * connection = (connection_t *)req->handle;

    Socket * socket = connection->socket;

    Buffer * buffer = socket->_write_buffers.Pop();

    if (status < 0)
    {
        if (socket->_server._logger)
            socket->_server._logger->Error("SocketServer::WriteCompletedCb() - %s", uv_strerror(status));
    }
    
    /*
     * Call to unqualified virtual function
     */
    socket->_server.WriteCompleted(socket, buffer, status);

    buffer->Release();

    socket->RemoveFlag(SocketOpt::F_WRITING);

    socket->TryWrite();

    if (socket->GetStatus() == SocketOpt::S_DISCONNECTING && !socket->HasFlag(SocketOpt::F_WRITING))
    {
        socket->Close();
    }

    socket->Release();
}

void SocketServer::ConnectionsCb(uv_prepare_t * handle)
{
    SocketServer * service = (SocketServer *)handle->loop->data;

    service->_req_list.Flush();

    size_t count = service->_req_list.Count();

    for (size_t i = 0; i < count; i++)
    {
        AppMessage & msg = service->_req_list[i];

        if (msg.msg_id == IO_Write_Request)
        {
            Socket * socket = (Socket *)msg.param1;
            Buffer * buffer = (Buffer *)msg.param2;

            socket->_write_buffers.Push(buffer);

            socket->TryWrite();

            if (msg.param3 == 1)
            {
                /*
                * final write, now shutdown send side of connection
                */
                socket->Shutdown();
            }

            socket->Release();
        }
        else if (msg.msg_id == IO_Close)
        {
            Socket * socket = (Socket *)msg.param1;

            socket->Shutdown();

            socket->Release();
        }
    }

    service->_req_list.Clear();
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
    ((connection_t *)socket)->socket = this;
    ((connection_t *)socket)->buffer = NULL;
}

SocketServer::Socket::~Socket()
{

}

void SocketServer::Socket::Attach(uv_tcp_t * socket)
{
    if (this->_socket)
        throw BaseException("SocketServer::Socket::Attach()", "socket already attached");

    this->_socket = socket;
    ((connection_t *)socket)->socket = this;
    ((connection_t *)socket)->buffer = NULL;

    this->Reset();
}

void SocketServer::Socket::Detatch()
{
    if (!this->_socket)
        throw BaseException("SocketServer::Socket::Detatch()", "socket is null");

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

void SocketServer::Socket::Shutdown()
{
    if (this->GetStatus() != SocketOpt::S_CONNECTED) return;

    this->SetStatus(SocketOpt::S_DISCONNECTING);

    if (this->HasFlag(SocketOpt::F_WRITING)) return;

    this->Close();
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

void SocketServer::Socket::AbortiveClose()
{
    this->_server.PostAbortiveClose(this);
}

void SocketServer::Socket::TryWrite()
{
    if (this->HasFlag(SocketOpt::F_WRITING)) return;

    if (this->_write_buffers.Empty()) return;

    if (this->HasFlag(SocketOpt::F_CLOSING))
    {
        while (!this->_write_buffers.Empty())
        {
            Buffer * buffer = this->_write_buffers.Pop();

            /*
            * Call to unqualified virtual function
            */
            this->_server.WriteCompleted(this, buffer, UV_ECANCELED);

            buffer->Release();
        }
        return;
    }

    Buffer * buffer = this->_write_buffers.Front();

    if (buffer->GetUsed() < sizeof(uv_write_t))
    {
        this->_write_buffers.PopFront();
        buffer->Release();
        this->TryWrite();
        return;
    }

    buffer->SetupWrite(sizeof(uv_write_t));

    this->AddRef();
    this->AddFlag(SocketOpt::F_WRITING);
    uv_write_t * req = (uv_write_t *)buffer->GetBuffer();
    int r = uv_write(req, (uv_stream_t *)this->_socket, buffer->GetUVBuffer(), 1, SocketServer::WriteCompletedCb);
    if (r != 0)
    {
        this->RemoveFlag(SocketOpt::F_WRITING);
        this->Release();
    }
}

void SocketServer::Socket::Close()
{
    if (this->HasFlag(SocketOpt::F_CLOSING)) return;

    this->AddFlag(SocketOpt::F_CLOSING);

    uv_close((uv_handle_t *)this->_socket, SocketServer::OnConnectionCloseCb);
}

void SocketServer::Socket::SetupRead(Buffer * buffer)
{
    this->AddFlag(SocketOpt::F_READING);

    ((connection_t *)this->_socket)->buffer = buffer;
}