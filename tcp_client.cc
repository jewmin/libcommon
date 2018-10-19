#include "tcp_client.h"

TcpClient::TcpClient(const char * name, size_t max_free_buffers, size_t buffer_size, Logger * logger)
    : Buffer::Allocator(buffer_size, max_free_buffers), logger_(logger), state_(kNew) {
    loop_.data = this;
    int err = uv_loop_init(&loop_);
    assert(0 == err);

    uv_sem_init(&thread_start_sem_, 0);

    STRNCPY_S(name_, name);
    memset(host_, 0, sizeof(host_));
    port_ = 0;

    recv_buffer_ = Allocate();
}

TcpClient::~TcpClient() {
    uv_sem_destroy(&thread_start_sem_);
    uv_loop_close(&loop_);

    recv_buffer_->Release();
    Flush();
}

int TcpClient::ConnectTo(const char * host, uint16_t port) {
    assert(kNew == state_);
    STRNCPY_S(host_, host);
    port_ = port;

    int err = ConnectToServer();
    if (0 == err) {
        err = Start();
    }

    if (0 == err) {
        uv_sem_wait(&thread_start_sem_);
        assert(kRunning == state_);
    }

    return err;
}

void TcpClient::InitiateShutdown() {
    uv_stop(&loop_);
}

void TcpClient::WaitForShutdownToComplete() {
    assert(kRunning == state_);
    QueueInLoop(std::bind(&TcpClient::InitiateShutdown, this));
    Stop();
}

void TcpClient::RunInLoop(const Functor & cb) {
    if (IsCurrentThread()) {
        cb();
    } else {
        QueueInLoop(cb);
    }
}

void TcpClient::QueueInLoop(const Functor & cb) {
    functor_lock_.Lock();
    pending_functors_.push_back(cb);
    functor_lock_.Unlock();

    int err = uv_async_send(&thread_req_);
    assert(0 == err);
}

void TcpClient::Run() {
    thread_req_.data = nullptr;
    int err = uv_async_init(&loop_, &thread_req_, ThreadReqCb);
    assert(0 == err);

    state_ = kRunning;
    uv_sem_post(&thread_start_sem_);
    uv_run(&loop_, UV_RUN_DEFAULT);
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
        tcp_.data = nullptr;
        err = uv_tcp_init(&loop_, uv_tcp());
    }

    if (0 == err) {
        err = uv_tcp_nodelay(uv_tcp(), GetNoDelay() ? 1 : 0);
    }

    if (0 == err) {
        if (GetKeepAlive() > 0) {
            err = uv_tcp_keepalive(uv_tcp(), 1, GetKeepAlive());
        } else {
            uv_tcp_keepalive(uv_tcp(), 0, 0);
        }
    }

    if (0 == err) {
        connect_req_.data = nullptr;
        err = uv_tcp_connect(&connect_req_, uv_tcp(), &s.addr, AfterConnect);
    }

    if (0 == err) {
        SetStatus(SocketOpt::S_CONNECTING);
    } else {
        if (logger_) {
            logger_->LogError("ConnectToServer error: %s", uv_strerror(err));
        }

        /*
         * Call to unqualified virtual function
         */
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
    if (0 == uv_is_closing(uv_handle())) {
        uv_read_stop(uv_stream());
        uv_close(uv_handle(), AfterClose);
    }
}

int TcpClient::SendInLoop(const char * data, size_t length, const WriteCallback & cb) {
    if (GetStatus() != SocketOpt::S_CONNECTED) {
        return UV_EPIPE;
    }

    WriteRequest * write_request = static_cast<WriteRequest *>(jc_malloc(sizeof(WriteRequest)));
    if (!write_request) {
        return UV_ENOMEM;
    }

    write_request->cb = cb;
    uv_buf_t buf = uv_buf_init(const_cast<char *>(data), static_cast<unsigned int>(length));
    int err = uv_write(&write_request->req, uv_stream(), &buf, 1, AfterWrite);
    if (0 != err) {
        if (logger_) {
            logger_->LogError("SendInLoop error: %s", uv_strerror(err));
        }
        jc_free(write_request);
    }

    return err;
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

    std::vector<Functor> functors;
    client->functor_lock_.Lock();
    functors.swap(client->pending_functors_);
    client->functor_lock_.Unlock();

    for (auto & func : functors) {
        func();
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

        /*
         * Call to unqualified virtual function
         */
        client->OnConnectFailed();
        client->Close();
    } else {
        client->SetStatus(SocketOpt::S_CONNECTED);
        client->AddFlag(SocketOpt::F_CONNECT);
        client->AddFlag(SocketOpt::F_READING);
        uv_read_start(client->uv_stream(), AllocBuffer, AfterRead);

        /*
         * Call to unqualified virtual function
         */
        client->OnConnected();
    }
}

void TcpClient::AfterClose(uv_handle_t * handle) {
    TcpClient * client = static_cast<TcpClient *>(handle->loop->data);
    if (!client) {
        return;
    }

    client->SetStatus(SocketOpt::S_DISCONNECTED);

    /*
     * Call to unqualified virtual function
     */
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

    client->recv_buffer_->SetupRead();
    *buf = *client->recv_buffer_->GetUVBuffer();
}

void TcpClient::AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
    TcpClient * client = static_cast<TcpClient *>(stream->loop->data);
    if (!client) {
        return;
    }

    if (client->GetStatus() != SocketOpt::S_CONNECTED) {
        return;
    }

    if (UV_EOF == nread || nread < 0) {
        if (client->logger_) {
            client->logger_->LogError("AfterRead error: %s", uv_strerror(static_cast<int>(nread)));
        }
        client->Shutdown();
    } else {
        client->recv_buffer_->Use(nread);

        /*
         * Call to unqualified virtual function
         */
        client->OnReadComplete(client->recv_buffer_);
        client->recv_buffer_->Empty();
    }
}

void TcpClient::AfterWrite(uv_write_t * req, int status) {
    WriteRequest * write_request = reinterpret_cast<WriteRequest *>(req);

    TcpClient * client = static_cast<TcpClient *>(write_request->req.handle->loop->data);
    if (client) {
        if (status < 0) {
            if (client->logger_) {
                client->logger_->LogError("AfterWrite error: %s", uv_strerror(status));
            }
            client->Shutdown();
        }

        if (write_request->cb) {
            write_request->cb(status);
        }
    }

    jc_free(write_request);
}