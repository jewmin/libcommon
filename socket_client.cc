#include "socket_client.h"
#include "exception.h"

enum IO_Operation
{
    IO_Read_Request,
    IO_Read_Completed,
    IO_Write_Request,
    IO_Write_Completed,
    IO_Close
};

SocketClient::SocketClient(size_t max_free_buffers, size_t buffer_size, ILog * logger)
    : BaseService(logger), Buffer::Allocator(buffer_size, max_free_buffers)
    , _is_connected(false), _is_connecting(false)
{
    this->_loop->data = this;
    uv_async_init(this->_loop, &this->_connect_event, SocketClient::ConnectionsCb);

    memset(this->_host, 0, sizeof(this->_host));
    this->_port = 0;
}

SocketClient::~SocketClient()
{
    try
    {
        this->StopConnections();
    }
    catch (...)
    {

    }
}

void SocketClient::ConnectTo(const char * host, uint16_t port)
{
    strncpy(this->_host, host, sizeof(this->_host));
    this->_port = port;

    this->Start();
}

void SocketClient::StartConnections()
{
    if (!this->_is_connected && !this->_is_connecting)
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
    if (this->_is_connected && !this->_is_connecting)
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

    Buffer * buffer = this->Allocate();

    this->PreWrite(buffer, data, data_length);

    buffer->AddData(data, data_length);

    AppMessage msg;
    msg.msg_id = IO_Write_Request;
    msg.param1 = (uint64_t)this;
    msg.param2 = (uint64_t)buffer;
    this->_msg_queue.Push(msg);
}

void SocketClient::Read(Buffer * buffer)
{
    if (!buffer)
        buffer = this->Allocate();
    else
        buffer->AddRef();

    this->_connect_socket.data = buffer;
}

void SocketClient::ReleaseBuffers()
{
    this->Flush();
}

void SocketClient::Run()
{
    try
    {
        BaseService::Run();
    }
    catch (const BaseException & ex)
    {
        if (this->_logger)
            this->_logger->Error("SocketClient::Run() - Exception: %s - %s", ex.Where(), ex.Message());
    }
    catch (...)
    {
        if (this->_logger)
            this->_logger->Error("SocketClient::Run() - Unexpected exception");
    }

    this->OnShutdownComplete();
}

void SocketClient::OnRecvMsg(uint32_t msg_id, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5)
{
    if (msg_id == IO_Write_Request)
    {
        SocketClient * socket = (SocketClient *)param1;
        Buffer * buffer = (Buffer *)param2;

        if (uv_is_writable((uv_stream_t *)&socket->_connect_socket))
        {
            buffer->SetupWrite();

            uv_write_t * req = (uv_write_t *)jc_malloc(sizeof(uv_write_t));
            req->data = buffer;
            uv_write(req, (uv_stream_t *)&socket->_connect_socket, buffer->GetUVBuffer(), 1, SocketClient::WriteCompletedCb);
        }
        else
        {
            if (socket->_logger)
                socket->_logger->Error("SocketClient::OnRecvMsg() - IO_Write_Request - socket handle is not writable");

            buffer->Release();
        }
    }
}

void SocketClient::ConnectionsCb(uv_async_t * handle)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    if (client->_is_connected)
    {
        uv_close((uv_handle_t *)&client->_connect_socket, SocketClient::OnCloseCb);
    }
    else
    {
        union {
            struct sockaddr addr;
            struct sockaddr_in addr4;
            struct sockaddr_in6 addr6;
        } s;

        int r;
        r = uv_ip4_addr(client->_host, client->_port, &s.addr4);
        if (r != 0)
            r = uv_ip6_addr(client->_host, client->_port, &s.addr6);
        
        if (r == 0)
            r = uv_tcp_init(client->_loop, &client->_connect_socket);

        if (r == 0)
            r = uv_tcp_connect(&client->_req, &client->_connect_socket, &s.addr, SocketClient::OnConnectCb);

        if (r != 0)
            throw BaseException("SocketClient::ConnectionsCb()", uv_strerror(r));
    }
    
    client->_is_connecting = true;
}

void SocketClient::OnConnectCb(uv_connect_t * req, int status)
{
    SocketClient * client = (SocketClient *)req->handle->loop->data;

    client->_is_connecting = false;

    if (status < 0)
    {
        if (client->_logger)
            client->_logger->Error("SocketClient::OnConnectCb() - %s", uv_strerror(status));
        
        client->_is_connected = false;

        client->OnConnectFail();
    }
    else
    {
        client->_is_connected = true;

        client->Read();
        uv_read_start((uv_stream_t *)&client->_connect_socket, SocketClient::AllocBufferCb, SocketClient::ReadCompletedCb);
        
        client->OnConnect();
    }
}

void SocketClient::OnCloseCb(uv_handle_t * handle)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    client->_is_connecting = false;

    client->_is_connected = false;

    client->OnClose();
}

void SocketClient::AllocBufferCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{
    SocketClient * client = (SocketClient *)handle->loop->data;

    if (!client->_is_connected) return;

    Buffer * buffer = (Buffer *)handle->data;

    buffer->SetupRead();

    *buf = *(buffer->GetUVBuffer());
}

void SocketClient::ReadCompletedCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
{
    SocketClient * client = (SocketClient *)stream->loop->data;

    if (!client->_is_connected) return;

    Buffer * buffer = (Buffer *)stream->data;

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
        {
            if (nread == UV_EOF)
                client->_logger->Info("SocketClient::ReadCompletedCb() - UV_EOF - client dropped");
            else
                client->_logger->Error("SocketClient::ReadCompletedCb() - %s", uv_strerror((int)nread));
        }

        client->StopConnections();
    }

    buffer->Release();
}

void SocketClient::WriteCompletedCb(uv_write_t * req, int status)
{
    SocketClient * client = (SocketClient *)req->handle->loop->data;

    if (!client->_is_connected) return;

    Buffer * buffer = (Buffer *)req->data;

    if (status < 0)
    {
        if (client->_logger)
            client->_logger->Error("SocketClient::WriteCompletedCb() - %s", uv_strerror(status));
    }
    
    /*
     * Call to unqualified virtual function
     */
    client->WriteCompleted(buffer);

    buffer->Release();
}