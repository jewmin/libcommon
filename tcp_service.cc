#include "tcp_service.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "tcp_connection.h"

TcpService::TcpService(uint32_t max_in_buffer_size, ILog * logger)
    : Super(logger)
{
    memset(this->_name, 0, sizeof(this->_name));
    this->_max_in_buffer_size = max_in_buffer_size;
    this->_handle.handle.data = this;
}

TcpService::~TcpService()
{

}

void TcpService::OnEstablished()
{
    this->OnConnected();
    uv_read_start(&this->_handle.stream, TcpService::AllocBufferCallback, TcpService::ReadCallback);
}

void TcpService::Close()
{
    this->OnClosing();
    uv_close(&this->_handle.handle, TcpService::CloseCallback);
}

void TcpService::OnClosing()
{

}

void TcpService::OnClosed()
{

}

void TcpService::OnTick()
{

}

void TcpService::OnConnected()
{

}

void TcpService::OnConnectFailed()
{

}

void TcpService::OnDisconnect()
{

}

void TcpService::OnDisconnected()
{

}

void TcpService::OnRecv(const char * data, int nread)
{

}

void TcpService::TimerCallback(uv_timer_t * handle)
{
    TcpService * service = (TcpService *)handle->data;
    service->OnTick();
}

void TcpService::ConnectionCallback(uv_stream_t * server, int status)
{
    TcpServer * service = (TcpServer *)server->data;

    if (status < 0)
    {
        if (service->_logger)
            service->_logger->Error("%s New Connection Error: %s", service->GetName(), uv_strerror(status));
        
        return;
    }

    TcpConnection * connection = service->NewConnection(*service);
    if (uv_accept(server, &connection->_handle.stream) == 0)
    {
        service->AddConnection(connection);
        connection->OnEstablished();
    }
    else
    {
        connection->Close();
    }
}

void TcpService::CloseCallback(uv_handle_t * handle)
{
    TcpService * service = (TcpService *)handle->data;
    service->OnClosed();
}

void TcpService::ConnectCallback(uv_connect_t * req, int status)
{
    TcpClient * service = (TcpClient *)req->handle->data;

    if (status < 0)
    {
        if (service->_logger)
            service->_logger->Error("%s Connect Error: %s", service->GetName(), uv_strerror(status));
        
        service->OnConnectFailed();
        return;
    }

    service->OnEstablished();
}

void TcpService::AllocBufferCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{
    TcpService * service = (TcpService *)handle->data;

    buf->base = (char *)malloc(service->_max_in_buffer_size);
    buf->len = service->_max_in_buffer_size;
}

void TcpService::ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
{
    TcpService * service = (TcpService *)stream->data;

    if (nread < 0)
    {
        if (nread != UV_EOF && service->_logger)
            service->_logger->Error("%s Read Error: %s", service->GetName(), uv_strerror(nread));
        
        service->Close();
    }
    else if (nread > 0)
        service->OnRecv(buf->base, nread);

    free(buf->base);
}