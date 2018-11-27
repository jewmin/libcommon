#include "tcp_server.h"

std::atomic_uint32_t TcpServer::s_num_created_ = 0;

TcpServer::TcpServer(EventLoop * loop, const char * name, const uint64_t tick_ms, const int max_out_buffer_size, const int max_in_buffer_size, const int max_free_sockets)
    : TcpSocket(loop, name, max_out_buffer_size, max_in_buffer_size)
    , destroy_(false), tick_timer_(0), tick_ms_(tick_ms), max_free_sockets_(max_free_sockets) {
    if (tick_ms_ > 0) {
        tick_timer_ = event_loop()->RunEvery(tick_ms_, std::bind(&TcpServer::OnTick, this));
    }
}

TcpServer::~TcpServer() {
    ReleaseSockets();
}

void TcpServer::Listen(const char * host, uint16_t port) {
    SetHost(host);
    SetPort(port);
    event_loop()->RunInLoop(std::bind(&TcpServer::ListenInLoop, this));
}

void TcpServer::Shutdown() {
    if (!destroy_) {
        destroy_ = true;
        if (tick_timer_ > 0) {
            event_loop()->Cancel(tick_timer_);
            tick_timer_ = 0;
        }
        TcpConnection * conn = active_list_.Head();
        while (conn) {
            TcpConnection * next_conn = SocketList::Next(conn);
            conn->Shutdown();
            conn = next_conn;
        }
        event_loop()->RunInLoop(std::bind(&TcpServer::ShutdownInLoop, this));
    }
}

TcpServer::TcpConnection * TcpServer::GetConnection(uint32_t index) {
    TcpConnection * conn = active_list_.Head();
    while (conn) {
        if (conn->index_ == index) {
            return conn;
        }
        conn = SocketList::Next(conn);
    }
    return nullptr;
}

void TcpServer::OnConnectFailed() {
    if (log()) {
        log()->LogInfo("%s server listen fail", name());
    }
}

void TcpServer::OnDisconnected() {
    if (log()) {
        log()->LogInfo("%s server disconnect", name());
    }
}

TcpSocket * TcpServer::AllocateSocket() {
    TcpConnection * conn = nullptr;

    if (!free_list_.IsEmpty()) {
        conn = free_list_.PopNode();
    } else {
        conn = new TcpConnection(*this);
    }

    active_list_.PushNode(conn);

    return conn;
}

void TcpServer::OnTick() {
    TcpConnection * conn = active_list_.Head();
    while (conn) {
        TcpConnection * next_conn = SocketList::Next(conn);
        OnTickEvent(conn);
        conn = next_conn;
    }
}

void TcpServer::ReleaseSockets() {
    while (active_list_.Head()) {
        ReleaseSocket(active_list_.Head());
    }

    while (free_list_.Head()) {
        DestroySocket(free_list_.PopNode());
    }
}

void TcpServer::ReleaseSocket(TcpConnection * conn) {
    conn->RemoveFromList();
    if (0 == max_free_sockets_ || free_list_.Count() < max_free_sockets_) {
        free_list_.PushNode(conn);
    } else {
        DestroySocket(conn);
    }
}

void TcpServer::DestroySocket(TcpConnection * conn) {
    delete conn;
}

/* TcpServer::TcpConnection implement */

TcpServer::TcpConnection::TcpConnection(TcpServer & server)
    : TcpSocket(server.event_loop(), server.name(), server.max_out_buffer_size_, server.max_in_buffer_size_)
    , server_(server), index_(++TcpServer::s_num_created_) {

}

TcpServer::TcpConnection::~TcpConnection() {

}

void TcpServer::TcpConnection::Shutdown() {
    event_loop()->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
}

void TcpServer::TcpConnection::OnConnected() {
    if (log()) {
        log()->LogInfo("%s connection succ", name());
    }
    server_.OnConnectionEstablished(this);
}

void TcpServer::TcpConnection::OnConnectFailed() {
    if (log()) {
        log()->LogInfo("%s connection fail", name());
    }
    server_.ReleaseSocket(this);
}

void TcpServer::TcpConnection::OnDisconnected() {
    if (log()) {
        log()->LogInfo("%s connection disconnect", name());
    }
    server_.OnConnectionClosed(this);
    server_.ReleaseSocket(this);
}

void TcpServer::TcpConnection::OnReadCompleted(const char * data, size_t size) {
    server_.OnReadCompleted(this, data, size);
}