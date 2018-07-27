#ifndef __ECHO_SERVER_ECHO_SERVER_H__
#define __ECHO_SERVER_ECHO_SERVER_H__

#include "socket_server.h"
#include <string>

class EchoServer : public SocketServer
{
public:
    EchoServer(const std::string & welcome_message, size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size = 1024, ILog * logger = NULL);
    virtual ~EchoServer();

private:
    virtual void OnStartAcceptingConnections();
    virtual void OnStopAcceptingConnections();
    virtual void OnShutdownInitiated();
    virtual void OnShutdownComplete();

    virtual void OnConnectionCreated();

    virtual void OnConnectionEstablished(SocketServer::Socket * socket, Buffer * address);

    virtual void OnConnectionClosed(SocketServer::Socket * socket);

    virtual void OnConnectionDestroyed();

    virtual void PreWrite(SocketServer::Socket * socket, Buffer * buffer, const char * data, size_t data_length);

    virtual void ReadCompleted(SocketServer::Socket * socket, Buffer * buffer);
    virtual void WriteCompleted(SocketServer::Socket * socket, Buffer * buffer);

    size_t GetMinimumMessageSize() const;
    size_t GetMessageSize(const Buffer * buffer) const;

    Buffer * ProcessDataStream(SocketServer::Socket * socket, Buffer * buffer) const;
    void ProcessCommand(SocketServer::Socket * socket, Buffer * buffer) const;

    /*
     * No copies do not implement
     */
    EchoServer(const EchoServer & rhs);
    EchoServer & operator =(const EchoServer & rhs);

private:
    const std::string _welcome_message;
};

#endif