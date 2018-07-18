#include "tcp_connection.h"

TcpConnection::TcpConnection(TcpServer & server)
    : Super(server._max_in_buffer_size, server._logger)
{
    this->_server = &server;
    uv_tcp_init(this->_server->_loop, &this->_handle.tcp);
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::OnClosing()
{
    this->OnDisconnect();
}

void TcpConnection::OnClosed()
{
    this->OnDisconnected();
    this->_server->DestroyConnection(this);
}

void TcpConnection::OnTick()
{

}

void TcpConnection::OnConnected()
{

}

void TcpConnection::OnConnectFailed()
{

}

void TcpConnection::OnDisconnect()
{

}

void TcpConnection::OnDisconnected()
{

}

void TcpConnection::OnRecv(const char * data, int nread)
{

}