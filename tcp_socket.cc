#include "tcp_socket.h"

TcpSocket::TcpSocket(EventLoop * loop, const char * name, const int max_out_buffer_size, const int max_in_buffer_size)
    : loop_(loop), max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {
    STRNCPY_S(name_, name);
    host_[0] = 0;
    port_ = 0;

    tcp_.data = this;
    connect_.data = nullptr;
}

TcpSocket::~TcpSocket() {

}

void TcpSocket::ListenInLoop() {
    if (flags_ & SocketOpt::F_LISTEN) {
        return;
    }

    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;

    int err = uv_ip4_addr(host_, port_, &s.addr4);
    if (0 != err) {
        err = uv_ip6_addr(host_, port_, &s.addr6);
    }

    if (0 == err) {
        err = uv_tcp_init(uv_loop(), uv_tcp());
    }

    if (0 == err) {
        err = uv_tcp_bind(uv_tcp(), &s.addr, 0);
    }

    if (0 == err) {
        err = uv_listen(uv_stream(), 128, NewConnection);
    }

    if (0 == err) {
        flags_ |= SocketOpt::F_LISTEN;
        status_ = SocketOpt::S_CONNECTED;
    } else {
        if (log()) {
            log()->LogError("(%s) ListenInLoop error: %s", name_, uv_strerror(err));
        }
        OnConnectFailed();
    }
}

void TcpSocket::ConnectInLoop() {
    if (SocketOpt::S_DISCONNECTED != status_) {
        return;
    }

    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;

    int err = uv_ip4_addr(host_, port_, &s.addr4);
    if (0 != err) {
        err = uv_ip6_addr(host_, port_, &s.addr6);
    }

    if (0 == err) {
        err = uv_tcp_init(uv_loop(), uv_tcp());
    }

    if (0 == err) {
        err = uv_tcp_nodelay(uv_tcp(), 1);
    }

    if (0 == err) {
        err = uv_tcp_connect(&connect_, uv_tcp(), &s.addr, AfterConnect);
    }

    if (0 == err) {
        status_ = SocketOpt::S_CONNECTING;
    } else {
        if (log()) {
            log()->LogError("(%s) ConnectInLoop error: %s", name_, uv_strerror(err));
        }
        OnConnectFailed();
    }
}

void TcpSocket::CloseInLoop() {
    if (!uv_is_closing(uv_handle())) {
        uv_close(uv_handle(), AfterClose);
    }
}

void TcpSocket::ShutdownInLoop() {
    if (SocketOpt::S_CONNECTED == status_) {
        status_ = SocketOpt::S_DISCONNECTING;
        ReadStopInLoop();
        CloseInLoop();
    }
}

void TcpSocket::ReadStartInLoop() {
    if (!(flags_ & SocketOpt::F_READING)) {
        flags_ |= SocketOpt::F_READING;
        recv_buffer_.Reserve(max_in_buffer_size_);
        uv_read_start(uv_stream(), AllocBuffer, AfterRead);
    }
}

void TcpSocket::ReadStopInLoop() {
    if (flags_ & SocketOpt::F_READING) {
        flags_ &= ~SocketOpt::F_READING;
        uv_read_stop(uv_stream());
    }
}

void TcpSocket::SendInLoop(const char * data, size_t size, bool assign, const WriteCompleteCallback & cb) {
    if (SocketOpt::S_CONNECTED == status_) {
        WriteRequest * request = new WriteRequest(this, data, size, assign, cb);
        int err = uv_write(&request->req_, uv_stream(), &request->buf_, 1, AfterWrite);
        if (err < 0) {
            if (log()) {
                log()->LogError("(%s) SendInLoop error: %s", name_, uv_strerror(err));
            }
            if (request->write_complete_cb_) {
                request->write_complete_cb_(err);
            }
            delete request;
        }
    }
}

int TcpSocket::AcceptInLoop(TcpSocket * accept_socket) {
    int err = uv_tcp_init(uv_loop(), accept_socket->uv_tcp());
    if (0 == err) {
        err = uv_accept(uv_stream(), accept_socket->uv_stream());
    }

    if (0 == err) {
        err = uv_tcp_nodelay(accept_socket->uv_tcp(), 1);
    }

    if (0 == err) {
        struct sockaddr_storage saddr;
        int saddr_len = sizeof(saddr);
        err = uv_tcp_getpeername(accept_socket->uv_tcp(), reinterpret_cast<struct sockaddr *>(&saddr), &saddr_len);
        if (0 == err) {
            if (AF_INET == saddr.ss_family) {
                char host[128] = { 0 };
                struct sockaddr_in * check_addr = reinterpret_cast<struct sockaddr_in *>(&saddr);
                uv_ip4_name(check_addr, host, sizeof(host));
                accept_socket->SetHost(host);
                accept_socket->SetPort(ntohs(check_addr->sin_port));
            } else if (AF_INET6 == saddr.ss_family) {
                char host[128] = { 0 };
                struct sockaddr_in6 * check_addr = reinterpret_cast<struct sockaddr_in6 *>(&saddr);
                uv_ip6_name(check_addr, host, sizeof(host));
                accept_socket->SetHost(host);
                accept_socket->SetPort(ntohs(check_addr->sin6_port));
            }
        }
    }

    return err;
}

void TcpSocket::NewConnection(uv_stream_t * stream, int status) {
    TcpSocket * socket = static_cast<TcpSocket *>(stream->data);
    if (socket) {
        if (status < 0) {
            if (socket->log()) {
                socket->log()->LogError("(%s) NewConnection error: %s", socket->name(), uv_strerror(status));
            }
            return;
        }

        TcpSocket * connection = socket->AllocateSocket();
        if (!connection) {
            if (socket->log()) {
                socket->log()->LogError("(%s) NewConnection error: connection is null", socket->name());
            }
            return;
        }

        int err = socket->AcceptInLoop(connection);
        if (0 != err) {
            if (socket->log()) {
                socket->log()->LogError("(%s) NewConnection error: %s", socket->name(), uv_strerror(err));
            }
            connection->CloseInLoop();
            connection->OnConnectFailed();
        } else {
            connection->status_ = SocketOpt::S_CONNECTED;
            connection->ReadStartInLoop();
            connection->OnConnected();
        }
    }
}

void TcpSocket::AfterConnect(uv_connect_t * req, int status) {
    TcpSocket * socket = static_cast<TcpSocket *>(req->handle->data);
    if (socket) {
        if (status < 0) {
            if (socket->log()) {
                socket->log()->LogError("(%s) AfterConnect error: %s", socket->name(), uv_strerror(status));
            }
            socket->status_ = SocketOpt::S_DISCONNECTED;
            socket->CloseInLoop();
            socket->OnConnectFailed();
        } else {
            socket->status_ = SocketOpt::S_CONNECTED;
            socket->ReadStartInLoop();
            socket->OnConnected();
        }
    }
}

void TcpSocket::AfterClose(uv_handle_t * handle) {
    TcpSocket * socket = static_cast<TcpSocket *>(handle->data);
    if (socket) {
        socket->flags_ = 0;
        socket->status_ = SocketOpt::S_DISCONNECTED;
        socket->OnDisconnected();
    }
}

void TcpSocket::AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
    TcpSocket * socket = static_cast<TcpSocket *>(handle->data);
    if (socket) {
        buf->base = reinterpret_cast<char *>(socket->recv_buffer_.GetOffsetPtr());
        buf->len = socket->recv_buffer_.GetWritableLength();
    }
}

void TcpSocket::AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
    TcpSocket * socket = static_cast<TcpSocket *>(stream->data);
    if (socket) {
        if (nread < 0) {
            if (socket->log()) {
                socket->log()->LogError("(%s) AfterRead error: %s", socket->name(), uv_strerror(static_cast<int>(nread)));
            }
            socket->ShutdownInLoop();
        } else {
            socket->recv_buffer_.AdjustOffset(nread);
            //socket->OnReadCompleted(reinterpret_cast<const char *>(socket->recv_buffer_.GetMemoryPtr()), socket->recv_buffer_.GetLength());
            //socket->recv_buffer_.SetLength(0);
            socket->OnReadCompleted(&socket->recv_buffer_);
        }
    }
}

void TcpSocket::AfterWrite(uv_write_t * req, int status) {
    WriteRequest * request = static_cast<WriteRequest *>(req->data);
    if (request) {
        if (status < 0) {
            if (request->socket_->log()) {
                request->socket_->log()->LogError("(%s) AfterWrite error: %s", request->socket_->name(), uv_strerror(status));
            }
        }
        if (request->write_complete_cb_) {
            request->write_complete_cb_(status);
        }
        delete request;
    }
}