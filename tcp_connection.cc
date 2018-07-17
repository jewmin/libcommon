#include "tcp_connection.h"

TcpConnection::TcpConnection(TcpServer & server)
    : Super(server._logger)
{
    this->_server = &server;
    uv_tcp_init(this->_server->_loop, &this->_handle.tcp);
}

TcpConnection::~TcpConnection()
{

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
    if (this->_logger)
        this->_logger->Debug("Tcp Connection Connected");
}

void TcpConnection::OnConnectFailed()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Connection Connect Failed");
}

void TcpConnection::OnDisconnect()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Connection Disconnect");
}

void TcpConnection::OnDisconnected()
{
    if (this->_logger)
        this->_logger->Debug("Tcp Connection Disconnected");
}

void TcpConnection::OnRecv(const char * data, int nread)
{
    
}