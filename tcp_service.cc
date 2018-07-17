#include "tcp_service.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "tcp_connection.h"

TcpService::TcpService(ILog * logger)
    : Super(logger)
{
    this->_handle.handle.data = this;
}

TcpService::~TcpService()
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
            service->_logger->Error("TCP Server New Connection Error: %s", uv_strerror(status));
        
        return;
    }

    TcpConnection * connection = service->NewConnection(*service);
    if (uv_accept(server, &connection->_handle.stream) == 0)
    {
        connection->OnConnected();
        uv_read_start(&connection->_handle.stream, TcpService::AllocBufferCallback, TcpService::ReadCallback);
    }
    else
    {
        connection->OnDisconnect();
        uv_close(&connection->_handle.handle, TcpService::CloseCallback);
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
            service->_logger->Error("TCP Client Connect Error: %s", uv_strerror(status));
        
        service->OnConnectFailed();
        return;
    }

    service->OnConnected();
    uv_read_start(&service->_handle.stream, TcpService::AllocBufferCallback, TcpService::ReadCallback);
}

void TcpService::AllocBufferCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void TcpService::ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
{
    TcpService * service = (TcpService *)stream->data;

    if (nread < 0)
    {
        if (nread != UV_EOF && service->_logger)
            service->_logger->Error("TCP Socket Read Error: %s", uv_strerror(nread));
        
        service->OnDisconnect();
        uv_close(&service->_handle.handle, TcpService::CloseCallback);
        free(buf->base);
        return;
    }

    if (nread > 0)
        service->OnRecv(buf->base, nread);
    free(buf->base);
}