#ifndef __ECHO_CLIENT_ECHO_CLIENT_H__
#define __ECHO_CLIENT_ECHO_CLIENT_H__

#include "tcp_client.h"

class EchoClient : public TcpClient {
public:
    EchoClient(EventLoop * loop);
    virtual ~EchoClient();

protected:
    virtual void OnConnected() override;
    virtual void OnTickEvent() override;
    virtual void OnReadCompleted(Packet * packet) override;

private:
    void SendRegisterClient();
    void AfterRegisterClient(int status);
    void SendMessage();
    void SendMessage(const char * message, size_t length);

    size_t GetMinimumMessageSize() const;
    size_t GetMessageSize(Packet * packet) const;
    void ProcessCommand(Packet * packet);

private:
    size_t send_total_;
    size_t recv_total_;
};

#endif