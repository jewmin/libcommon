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
        uv_read_start(&connection->_handle.stream, TcpService::AllocBufferCallback, TcpService::ReadCallback);
        connection->OnConnected();
    }
    else
    {
        uv_close(&connection->_handle.handle, TcpService::CloseCallback);
    }
}

void TcpService::CloseCallback(uv_handle_t * handle)
{
    TcpConnection * connection = (TcpConnection *)handle->data;
    connection->Close();
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

    uv_read_start(&service->_handle.stream, TcpService::AllocBufferCallback, TcpService::ReadCallback);
    service->OnConnected();
}

void TcpService::AllocBufferCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf)
{

}

// void TcpService::ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf)
// {
//     TcpService * service = (TcpService *)stream->data;

//     if (nread < 0)
//     {
//         if (service->_logger)
//             service->_logger->Error("TCP Socket Read Error: %s", uv_strerror(nread));
        
//         return;
//     }
//     else if (nread > 0)
//     {
//         service->OnRecv(buf->base, nread);
//     }
// }