#include "tcp_client.h"

TcpClient::TcpClient(size_t max_free_buffers, size_t buffer_size, Logger * logger)
    : Buffer::Allocator(buffer_size, max_free_buffers), logger_(logger)
    , state_(kNew), send_buffer_len_(0) {
    loop_.data = this;
    int err = uv_loop_init(&loop_);
    assert(0 == err);

    uv_sem_init(&thread_start_sem_, 0);

    memset(host_, 0, sizeof(host_));
    port_ = 0;

    recv_buffer_ = Allocate();
    send_buffer_ = Allocate();
}

TcpClient::~TcpClient() {
    uv_sem_destroy(&thread_start_sem_);
    uv_loop_close(&loop_);

    recv_buffer_->Release();
    send_buffer_->Release();
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
    AppendMessage(kStop);
    Stop();
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
        err = uv_tcp_init(&loop_, uv_tcp());
        tcp_.data = nullptr;
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
        err = uv_tcp_connect(&connect_req_, uv_tcp(), &s.addr, AfterConnect);
        connect_req_.data = nullptr;
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
    if (0 == uv_is_closing(uv_handle())) {
        uv_read_stop(uv_stream());
        uv_close(uv_handle(), AfterClose);
    }
}

bool TcpClient::SendData(const char * data, size_t length) {
    if (!data || length == 0) {
        return false;
    }

    if (GetStatus() != SocketOpt::S_CONNECTED) {
        return false;
    }

    {
        Mutex::Guard guard(send_buffer_lock_);

        if (HasFlag(SocketOpt::F_WRITING)) {
            return false;
        }

        AddFlag(SocketOpt::F_WRITING);
    }

    if (IsCurrentThread()) {
        Send(data, length);
    } else {
        size_t avaliable = send_buffer_->GetSize() - send_buffer_->GetUsed();
        if (avaliable > length) {
            avaliable = length;
        }
        send_buffer_->AddData(data, avaliable);
        AppendMessage(kSendMessage);
    }

    return true;
}

void TcpClient::Send(const char * data, size_t length) {
    uv_buf_t buf = uv_buf_init(const_cast<char *>(data), static_cast<unsigned int>(length));
    int err = uv_write(&write_req_, uv_stream(), &buf, 1, AfterWrite);
    if (0 != err) {
        if (logger_) {
            logger_->LogError("Send error: %s", uv_strerror(err));
        }
        RemoveFlag(SocketOpt::F_WRITING);
        OnWriteComplete(err);
    } else {
        send_buffer_len_ = length;
    }
}

void TcpClient::AppendMessage(action_t action) {
    outgoing_message_queue_.Push(kSendMessage);
    int err = uv_async_send(&thread_req_);
    assert(err == 0);
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

    client->outgoing_message_queue_.Flush();
    for (int i = 0; i < client->outgoing_message_queue_.Count(); ++i) {
        action_t action = client->outgoing_message_queue_[i];
        switch (action) {
        case kSendMessage:
            client->Send(reinterpret_cast<const char *>(client->send_buffer_->GetBuffer()), client->send_buffer_->GetUsed());
            client->send_buffer_->Empty();
            break;

        case kStop:
            uv_stop(&client->loop_);
            break;
        }
    }
    client->outgoing_message_queue_.Clear();
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
        uv_read_start(client->uv_stream(), AllocBuffer, AfterRead);
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
        client->OnReadComplete(client->recv_buffer_);
    }
}

void TcpClient::AfterWrite(uv_write_t * req, int status) {
    TcpClient * client = static_cast<TcpClient *>(req->handle->loop->data);
    if (!client) {
        return;
    }

    if (status < 0)
    {
        if (client->logger_) {
            client->logger_->LogError("AfterWrite error: %s", uv_strerror(status));
        }
        client->Shutdown();
    }

    int err = status < 0 ? status : static_cast<int>(client->send_buffer_len_);
    client->send_buffer_len_ = 0;
    client->RemoveFlag(SocketOpt::F_WRITING);
    client->OnWriteComplete(err);
}