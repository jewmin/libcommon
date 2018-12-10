#ifndef __ECHO_SERVER_ECHO_SERVER_H__
#define __ECHO_SERVER_ECHO_SERVER_H__

#include "tcp_server.h"

class EchoServer : public TcpServer {
public:
    EchoServer(const std::string & welcome_message, EventLoop * loop);
    virtual ~EchoServer();

protected:
    virtual void OnTickEvent(TcpConnection * conn) override;
    virtual void OnShutdownInitiated() override;
    virtual void OnReadCompleted(TcpConnection * conn, Packet * packet) override;

private:
    void OnStatisticsTick();
    void SendMessage(TcpConnection * conn);
    void SendMessage(TcpConnection * conn, const char * message, size_t length);

    size_t GetMinimumMessageSize() const;
    size_t GetMessageSize(Packet * packet) const;
    void ProcessCommand(Packet * packet);

private:
    const std::string welcome_message_;
    uint32_t statistics_timer_;
    uint32_t run_second_;
    uint32_t send_total_;
    uint32_t recv_total_;
    uint32_t current_second_send_;
    uint32_t current_second_recv_;
    uint32_t last_second_send_;
    uint32_t last_second_recv_;
    uint32_t high_send_pre_second_;
    uint32_t high_recv_pre_second_;
    double recv_average_;
    double send_average_;
};

#endif