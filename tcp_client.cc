#include "tcp_client.h"

TcpClient::TcpClient(size_t max_free_buffers, size_t buffer_size, Logger * logger)
    : Buffer::Allocator(buffer_size, max_free_buffers), logger_(logger)
    , state_(kNew), recv_buffer_(nullptr), send_buffer_(nullptr) {
    loop_.data = this;
    int err = uv_loop_init(&loop_);
    assert(0 == err);

    uv_sem_init(&thread_start_sem_, 0);

    memset(host_, 0, sizeof(host_));
    port_ = 0;
}

TcpClient::~TcpClient() {
    uv_sem_destroy(&thread_start_sem_);
    uv_loop_close(&loop_);
}

int TcpClient::ConnectTo(const char * host, uint16_t port) {
    assert(kNew == state_);
    strncpy(host_, host, sizeof(host_));
    port_ = port;

    int err = ConnectToServer();
    if (0 == err) {
        err = Start();
    }

    if (0 == err) {
        uv_sem_wait(&thread_start_sem_);
    }

    return err;
}

void TcpClient::WaitForShutdownToComplete() {
    assert(kRunning == state_);
    Stop();
}

void TcpClient::Run() {
    thread_req_.data = nullptr;
    int err = uv_async_init(&loop_, &thread_req_, ThreadReqCb);
    assert(0 == err);

    state_ = kRunning;
    uv_sem_post(&thread_start_sem_);
    while (!IsTerminated()) {
        uv_run(&loop_, UV_RUN_ONCE);
        jc_sleep(1);
    }
}

void TcpClient::OnTerminated() {
    Shutdown();
    uv_close(reinterpret_cast<uv_handle_t *>(&thread_req_), nullptr);
    while (uv_run(&loop_, UV_RUN_NOWAIT)) {
        uv_run(&loop_, UV_RUN_ONCE);
    }
    state_ = kDone;
}

int TcpClient::ConnectToServer() {
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
        err = uv_tcp_init(&loop_, tcp());
        tcp_.data = nullptr;
    }

    if (0 == err) {
        err = uv_tcp_nodelay(tcp(), GetNoDelay() ? 1 : 0);
    }

    if (0 == err) {
        if (GetKeepAlive() > 0) {
            err = uv_tcp_keepalive(tcp(), 1, GetKeepAlive());
        } else {
            uv_tcp_keepalive(tcp(), 0, 0);
        }
    }

    if (0 == err) {
        err = uv_tcp_connect(&connect_, tcp(), &s.addr, AfterConnect);
        connect_.data = nullptr;
    }

    if (0 == err) {
        SetStatus(SocketOpt::S_CONNECTING);
    } else {
        if (logger_) {
            logger_->LogError("ConnectToServer error: %s", uv_strerror(err));
        }

        OnConnectFailed();
    }

    return err;
}

void TcpClient::Shutdown() {
    if (GetStatus() == SocketOpt::S_CONNECTED) {
        return;
    }

    SetStatus(SocketOpt::S_DISCONNECTING);
    Close();
}

void TcpClient::Close() {
    if (HasFlag(SocketOpt::F_CLOSING)) {
        return;
    }

    AddFlag(SocketOpt::F_CLOSING);
    if (0 == uv_is_closing(handle())) {
        uv_read_stop(stream());
        uv_close(handle(), AfterClose);
    }
}

void TcpClient::OnConnected() {

}

void TcpClient::OnConnectFailed() {

}

void TcpClient::OnDisconnected() {

}

void TcpClient::ThreadReqCb(uv_async_t * handle) {
    TcpClient * client = static_cast<TcpClient *>(handle->loop->data);
    if (!client) {
        return;
    }
}

void TcpClient::AfterConnect(uv_connect_t * req, int status) {
    TcpClient * client = static_cast<TcpClient *>(req->handle->loop->data);
    if (!client) {
        return;
    }

    client->ClearFlag();

    if (status < 0) {
        if (client->logger_) {
            client->logger_->LogError("AfterConnect error: %s", uv_strerror(status));
        }

        client->OnConnectFailed();
        client->Close();
    } else {
        client->SetStatus(SocketOpt::S_CONNECTED);
        client->AddFlag(SocketOpt::F_CONNECT);
        client->AddFlag(SocketOpt::F_READING);
        client->OnConnected();
        uv_read_start(client->stream(), AllocBuffer, AfterRead);
        /*if (!client->send_buffer_) {
            client->send_buffer_ = client->Allocate();
        }*/
    }
}

void TcpClient::AfterClose(uv_handle_t * handle) {
    TcpClient * client = static_cast<TcpClient *>(handle->loop->data);
    if (!client) {
        return;
    }

    client->SetStatus(SocketOpt::S_DISCONNECTED);
    client->OnDisconnected();
}

void TcpClient::AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
    TcpClient * client = static_cast<TcpClient *>(handle->loop->data);
    if (!client) {
        return;
    }

    if (client->GetStatus() != SocketOpt::S_CONNECTED) {
        return;
    }

    if (!client->recv_buffer_) {
        client->recv_buffer_ = client->Allocate();
    }

    client->recv_buffer_->SetupRead();
    *buf = *client->recv_buffer_->GetUVBuffer();
}

void TcpClient::AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
    TcpClient * client = static_cast<TcpClient *>(stream->loop->data);
    if (!client) {
        return;
    }

    if (!client->recv_buffer_) {
        return;
    }

    if (client->GetStatus() != SocketOpt::S_CONNECTED) {
        client->recv_buffer_->Release();
        client->recv_buffer_ = nullptr;
        return;
    }

    if (UV_EOF == nread || nread < 0) {
        if (client->logger_) {
            client->logger_->LogError("AfterRead error: %s", uv_strerror(static_cast<int>(nread)));
        }
        client->Shutdown();
    } else {
        client->recv_buffer_->Use(nread);
        client->OnRead(client->recv_buffer_);
    }

    client->recv_buffer_->Release();
    client->recv_buffer_ = nullptr;
}