#include "socket_client.h"
#include "exception.h"

SocketClient::SocketClient(size_t max_free_buffers, size_t buffer_size, ILog * logger)
    : BaseService(logger), Buffer::Allocator(buffer_size, max_free_buffers)
{
    this->_loop->data = this;
    uv_async_init(this->_loop, &this->_connect_event, SocketClient::ConnectionsCb);
    uv_async_init(this->_loop, &this->_write_notify_event, NULL);
    uv_prepare_init(this->_loop, &this->_write_event);

    memset(this->_host, 0, sizeof(this->_host));
    this->_port = 0;
}

SocketClient::~SocketClient()
{
    
}

int SocketClient::ConnectTo(const char * host, uint16_t port)
{
    strncpy(this->_host, host, sizeof(this->_host));
    this->_port = port;

    return this->Start();
}

void SocketClient::StartConnections()
{
    if (this->GetStatus() == SocketOpt::S_DISCONNECTED)
    {
        /*
         * Call to unqualified virtual function
         */
        this->OnStartConnections();

        uv_async_send(&this->_connect_event);
    }
}

void SocketClient::StopConnections()
{
    if (this->GetStatus() == SocketOpt::S_CONNECTED)
    {
        uv_async_send(&this->_connect_event);

        /*
         * Call to unqualified virtual function
         */
        this->OnStopConnections();
    }
}

void SocketClient::InitiateShutdown()
{
    this->StopConnections();

    /*
     * Call to unqualified virtual function
     */
    this->OnShutdownInitiated();
}

void SocketClient::WaitForShutdownToComplete()
{
    this->InitiateShutdown();

    this->Stop();
}

void SocketClient::Write(const char * data, size_t data_length)
{
    if (!data || data_length == 0) return;

    if (this->GetStatus() != SocketOpt::S_CONNECTED) return;

    Buffer * buffer = this->Allocate();

    buffer->Use(sizeof(uv_write_t));

    this->PreWrite(buffer, data, data_length);

    if (buffer->GetSize() < buffer->GetUsed() || buffer->GetSize() - buffer->GetUsed() < data_length)
    {
        if (this->_logger)
            this->_logger->Error("SocketClient::Write() - %s", uv_strerror(UV_ENOBUFS));

        buffer->Release();
        return;
    }

    buffer->AddData(data, data_length);

    this->_write_buffers.Push(buffer);
    uv_async_send(&this->_write_notify_event);
}

void SocketClient::Read(Buffer * buffer)
{
    if (!buffer)
        buffer = this->Allocate();
    else
        buffer->AddRef();

    this->_connect_socket.data = buffer;
}

void SocketClient::TryWrite()
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
            this->WriteCompleted(buffer, UV_ECANCELED);

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

    this->AddFlag(SocketOpt::F_WRITING);
    uv_write_t * req = (uv_write_t *)buffer->GetBuffer();
    int r = uv_write(req, (uv_stream_t *)&this->_connect_socket, buffer->GetUVBuffer(), 1, SocketClient::WriteCompletedCb);
    if (r != 0) this->RemoveFlag(SocketOpt::F_WRITING);
}

void SocketClient::Shutdown()
{
    if (this->GetStatus() != SocketOpt::S_CONNECTED) return;

    this->SetStatus(SocketOpt::S_DISCONNECTING);

    if (this->HasFlag(SocketOpt::F_WRITING)) return;

    this->Close();
}

void SocketClient::Close()
{
    if (this->HasFlag(SocketOpt::F_CLOSING)) return;

    this->AddFlag(SocketOpt::F_CLOSING);

    uv_close((uv_handle_t *)&this->_connect_socket, SocketClient::OnCloseCb);
}

void SocketClient::Connect()
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
        r = uv_tcp_init(this->_loop, &this->_connect_socket);

    if (r == 0)
        r = uv_tcp_nodelay(&this->_connect_socket, this->GetNoDelay() ? 1 : 0);

    if (r == 0)
    {
        if (this->GetKeepAlive() > 0)
            r = uv_tcp_keepalive(&this->_connect_socket, 1, this->GetKeepAlive());
        else
            r = uv_tcp_keepalive(&this->_connect_socket, 0, 0);
    }

    if (r == 0)
        r = uv_tcp_connect(&this->_req, &this->_connect_socket, &s.addr, SocketClient::OnConnectCb);

    if (r == 0)
    {
        this->SetStatus(SocketOpt::S_CONNECTING);
    }
    else
    {
        if (this->_logger)
            this->_logger->Error("SocketClient::Connect() - %s", uv_strerror(r));
        
        /*
        * Call to unqualified virtual function
        */
        this->OnConnectFail();
    }
}

void SocketClient::ReleaseBuffers()
{
    this->Flush();
}

void SocketClient::Run()
{
    uv_prepare_start(&this->_write_event, SocketClient::TryWriteCb);
    uv_prepare_start(&this->_msg_handle, BaseService::MsgCallback);
    uv_run(this->_loop, UV_RUN_DEFAULT);
    while (this->GetStatus() != SocketOpt::S_DISCONNECTED) uv_run(this->_loop, UV_RUN_ONCE);
    this->ReleaseBuffers();

    /*
    * Call to unqualified virtual function
    */
    this->OnShutdownComplete();
}

void SocketClient::ConnectionsCb(uv_async_t * handle)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    if (client->GetStatus() == SocketOpt::S_CONNECTED)
    {
        client->Shutdown();
    }
    else
    {
        client->Connect();
    }
}

void SocketClient::OnConnectCb(uv_connect_t * req, int status)
{
    SocketClient * client = (SocketClient *)req->handle->loop->data;

    client->ClearFlag();

    if (status < 0)
    {
        if (client->_logger)
            client->_logger->Error("SocketClient::OnConnectCb() - %s", uv_strerror(status));
        
        /*
        * Call to unqualified virtual function
        */
        client->OnConnectFail();

        client->Close();
    }
    else
    {
        client->SetStatus(SocketOpt::S_CONNECTED);

        client->AddFlag(SocketOpt::F_CONNECT);

        /*
        * Call to unqualified virtual function
        */
        client->OnConnect();
        
        client->Read();

        uv_read_start((uv_stream_t *)&client->_connect_socket, SocketClient::AllocBufferCb, SocketClient::ReadCompletedCb);
    }
}

void SocketClient::OnCloseCb(uv_handle_t * handle)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    client->SetStatus(SocketOpt::S_DISCONNECTED);

    /*
    * Call to unqualified virtual function
    */
    client->OnClose();
}

void SocketClient::AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    if (client->GetStatus() != SocketOpt::S_CONNECTED) return;

    Buffer * buffer = (Buffer *)handle->data;

    buffer->SetupRead();

    *buf = *(buffer->GetUVBuffer());
}

void SocketClient::ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
{
    SocketClient * client = (SocketClient *)stream->loop->data;

    Buffer * buffer = (Buffer *)stream->data;

    if (client->GetStatus() != SocketOpt::S_CONNECTED)
    {
        buffer->Release();
        uv_read_stop(stream);
        return;
    }

    if (nread >= 0)
    {
        buffer->Use(nread);

        /*
         * Call to unqualified virtual function
         */
        client->ReadCompleted(buffer);
    }
    else
    {
        if (client->_logger)
            client->_logger->Error("SocketClient::ReadCompletedCb() - %s", uv_strerror((int)nread));

        client->Shutdown();
    }

    buffer->Release();
}

void SocketClient::WriteCompletedCb(uv_write_t * req, int status)
{
    SocketClient * client = (SocketClient *)req->handle->loop->data;

    Buffer * buffer = client->_write_buffers.Pop();

    if (status < 0)
    {
        if (client->_logger)
            client->_logger->Error("SocketClient::WriteCompletedCb() - %s", uv_strerror(status));
    }
    
    /*
     * Call to unqualified virtual function
     */
    client->WriteCompleted(buffer, status);

    buffer->Release();

    client->RemoveFlag(SocketOpt::F_WRITING);

    client->TryWrite();

    if (client->GetStatus() == SocketOpt::S_DISCONNECTING && !client->HasFlag(SocketOpt::F_WRITING))
    {
        client->Close();
    }
}

void SocketClient::TryWriteCb(uv_prepare_t * handle)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    client->TryWrite();
}