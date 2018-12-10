#include "tcp_client.h"

TcpClient::TcpClient(EventLoop * loop, const char * name, uint64_t tick_ms, uint64_t reconnect_ms, const int max_out_buffer_size, const int max_in_buffer_size)
    : TcpSocket(loop, name, max_out_buffer_size, max_in_buffer_size), SendPacketPool(this)
    , destroy_(false), tick_timer_(0), tick_ms_(tick_ms), reconnect_ms_(reconnect_ms) {
    if (tick_ms_ > 0) {
        tick_timer_ = event_loop()->RunEvery(tick_ms_, std::bind(&TcpClient::OnTick, this));
    }
}

TcpClient::~TcpClient() {

}

void TcpClient::Connect(const char * host, uint16_t port) {
    SetHost(host);
    SetPort(port);
    event_loop()->RunInLoop(std::bind(&TcpClient::ConnectInLoop, this));
}

void TcpClient::Shutdown() {
    if (!destroy_) {
        destroy_ = true;
        if (tick_timer_ > 0) {
            event_loop()->Cancel(tick_timer_);
            tick_timer_ = 0;
        }
        OnShutdownInitiated();
        event_loop()->RunInLoop(std::bind(&TcpClient::ShutdownInLoop, this));
    }
}

void TcpClient::OnConnected() {
    if (log()) {
        log()->LogInfo("%s server succ", name());
    }
}

void TcpClient::OnConnectFailed() {
    if (log()) {
        log()->LogInfo("%s server fail", name());
    }
    if (!destroy_ && reconnect_ms_ > 0) {
        event_loop()->RunAfter(reconnect_ms_, std::bind(&TcpClient::ConnectInLoop, this));
    }
}

void TcpClient::OnDisconnected() {
    if (log()) {
        log()->LogInfo("%s server disconnect", name());
    }
    if (!destroy_ && reconnect_ms_ > 0) {
        event_loop()->RunAfter(reconnect_ms_, std::bind(&TcpClient::ConnectInLoop, this));
    }
}

void TcpClient::OnTick() {
    if (SocketOpt::S_CONNECTED == status()) {
        OnTickEvent();
    }
}