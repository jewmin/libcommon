#include "tcp_connection.h"

TcpConnection::TcpConnection(TcpServer & server)
    : Super(server._max_in_buffer_size, server._logger)
{
    this->_index = ++server._generate_id;

    snprintf(this->_name, sizeof(this->_name) - 1, "%s_%u", server._name, this->_index);
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
    this->_server->RemoveConnection(this);
    this->_server->DestroyConnection(this);
}

void TcpConnection::OnTick()
{

}

void TcpConnection::OnConnected()
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