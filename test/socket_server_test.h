#ifndef __UNIT_TEST_SOCKET_SERVER_TEST_H__
#define __UNIT_TEST_SOCKET_SERVER_TEST_H__

#include "gmock/gmock.h"
#include "socket_server.h"

class MockSocketServer : public SocketServer
{
public:
    MockSocketServer(size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024)
        : SocketServer(max_free_sockets, max_free_buffers, buffer_size)
    {
        this->_last_socket = NULL;
    }

    virtual ~MockSocketServer()
    {

    }

    void Close()
    {
        if (this->_last_socket)
            this->_last_socket->AbortiveClose();
    }

private:
    virtual void OnConnectionEstablished(SocketServer::Socket * socket, Buffer * address)
    {
        this->_last_socket = socket;
    }

    virtual void OnConnectionClosed(SocketServer::Socket * socket)
    {
        if (this->_last_socket == socket)
            this->_last_socket = NULL;
    }

    virtual void ReadCompleted(SocketServer::Socket * socket, Buffer * buffer)
    {
        socket->Write(buffer, true);
        socket->Read();
    }

    virtual void WriteCompleted(SocketServer::Socket * socket, Buffer * buffer)
    {

    }

    /*
     * No copies do not implement
     */
    MockSocketServer(const MockSocketServer & rhs);
    MockSocketServer & operator =(const MockSocketServer & rhs);

public:
    SocketServer::Socket * _last_socket;
};

#endif