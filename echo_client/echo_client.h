#ifndef __ECHO_CLIENT_ECHO_CLIENT_H__
#define __ECHO_CLIENT_ECHO_CLIENT_H__

#include "socket_client.h"

class EchoClient : public SocketClient
{
public:
    EchoClient(size_t max_free_buffers, size_t buffer_size = 1024, ILog * logger = NULL);
    virtual ~EchoClient();

private:
    virtual void OnStartConnections();
    virtual void OnStopConnections();
    virtual void OnShutdownInitiated();
    virtual void OnShutdownComplete();

    virtual void OnConnect();
    virtual void OnConnectFail();
    virtual void OnClose();

    virtual void ReadCompleted(Buffer * buffer);
    virtual void WriteCompleted(Buffer * buffer, int status);

    virtual void PreWrite(Buffer * buffer, const char * data, size_t data_length);

    size_t GetMinimumMessageSize() const;
    size_t GetMessageSize(const Buffer * buffer) const;

    Buffer * ProcessDataStream(Buffer * buffer);
    void ProcessCommand(Buffer * buffer);
};

#endif