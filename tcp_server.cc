#include "tcp_server.h"
#include "exception.h"

TcpServer::TcpServer(const char * name, size_t max_free_sockets, size_t max_free_buffers, size_t buffer_size, Logger * logger)
    : Buffer::Allocator(buffer_size, max_free_buffers)
    , logger_(logger), max_free_sockets_(max_free_sockets), state_(kNew) {
    loop_.data = this;
    int err = uv_loop_init(&loop_);
    assert(0 == err);

    uv_sem_init(&thread_start_sem_, 0);

    STRNCPY_S(name_, name);
    memset(host_, 0, sizeof(host_));
    port_ = 0;
}

TcpServer::~TcpServer() {
    ReleaseSockets();
    uv_sem_destroy(&thread_start_sem_);
    uv_loop_close(&loop_);

    Flush();
}

int TcpServer::Listen(const char * host, uint16_t port) {
    assert(kNew == state_);
    STRNCPY_S(host_, host);
    port_ = port;

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
        err = uv_tcp_bind(uv_tcp(), &s.addr, 0);
    }

    if (0 == err) {
        err = uv_listen(uv_stream(), 128, NewConnection);
    }

    if (0 != err) {
        if (logger_) {
            logger_->LogError("Listen error: %s", uv_strerror(err));
        }
    }

    if (0 == err) {
        err = Start();
    }
    
    if (0 == err) {
        uv_sem_wait(&thread_start_sem_);
        assert(kRunning == state_);
    }

    return err;
}

void TcpServer::WaitForShutdownToComplete() {
    assert(kRunning == state_);
    AppendMessage(kStop, nullptr);
    Stop();
}

void TcpServer::Run() {
    thread_req_.data = nullptr;
    int err = uv_async_init(&loop_, &thread_req_, ThreadReqCb);
    assert(0 == err);

    state_ = kRunning;
    uv_sem_post(&thread_start_sem_);
    uv_run(&loop_, UV_RUN_DEFAULT);
}

void TcpServer::OnTerminated() {
    uv_close(uv_handle(), nullptr);
    uv_close(reinterpret_cast<uv_handle_t *>(&thread_req_), nullptr);
    ReleaseSockets();
    while (uv_run(&loop_, UV_RUN_NOWAIT)) {
        uv_run(&loop_, UV_RUN_ONCE);
    }
    state_ = kDone;
}

TcpServer::Socket * TcpServer::AllocateSocket() {
    Socket * socket = nullptr;

    if (!free_list_.IsEmpty()) {
        socket = free_list_.PopNode();
        socket->AddRef();
    } else {
        socket = new Socket(*this);

        /*
         * Call to unqualified virtual function
         */
        OnConnectionCreated();
    }

    active_list_.PushNode(socket);

    return socket;
}

void TcpServer::ReleaseSocket(Socket * socket) {
    if (!socket) {
        throw BaseException("TcpServer::ReleaseSocket()", "socket is null");
    }

    socket->RemoveFromList();

    if (0 == max_free_sockets_ || free_list_.Count() < max_free_sockets_) {
        socket->Clear();
        free_list_.PushNode(socket);
    } else {
        DestroySocket(socket);
    }
}

void TcpServer::DestroySocket(Socket * socket) {
    delete socket;

    /*
     * Call to unqualified virtual function
     */
    OnConnectionDestroyed();
}

void TcpServer::ReleaseSockets() {
    Socket * socket = active_list_.Head();

    while (socket) {
        Socket * next = SocketList::Next(socket);
        socket->Shutdown();
        socket = next;
    }

    while (!active_list_.IsEmpty()) {
        uv_run(&loop_, UV_RUN_ONCE);
    }

    while (free_list_.Head()) {
        DestroySocket(free_list_.PopNode());
    }
}

void TcpServer::AppendMessage(action_t action, Socket * socket) {
    outgoing_message_queue_.Push(std::make_tuple(action, socket));
    int err = uv_async_send(&thread_req_);
    assert(0 == err);
}

void TcpServer::OnConnectionCreated() {

}

void TcpServer::OnConnectionClosed(Socket * socket) {

}

void TcpServer::OnConnectionDestroyed() {

}

void TcpServer::ThreadReqCb(uv_async_t * handle) {
    TcpServer * server = static_cast<TcpServer *>(handle->loop->data);
    if (!server) {
        return;
    }

    server->outgoing_message_queue_.Flush();
    for (int i = 0; i < server->outgoing_message_queue_.Count(); ++i) {
        auto & outgoing = server->outgoing_message_queue_[i];
        switch (std::get<0>(outgoing)) {
        case kSendMessage: {
            Socket * socket = std::get<1>(outgoing);
            socket->Send(reinterpret_cast<const char *>(socket->send_buffer_->GetBuffer()), socket->send_buffer_->GetUsed());
            socket->send_buffer_->Empty();
            break;
        }

        case kKill: {
            Socket * socket = std::get<1>(outgoing);
            socket->Shutdown();
            break;
        }

        case kStop:
            uv_stop(&server->loop_);
            break;
        }
    }
    server->outgoing_message_queue_.Clear();
}

void TcpServer::NewConnection(uv_stream_t * stream, int status) {
    TcpServer * server = static_cast<TcpServer *>(stream->loop->data);
    if (!server) {
        return;
    }

    if (status < 0) {
        if (server->logger_) {
            server->logger_->LogError("NewConnection error: %s", uv_strerror(status));
        }
        return;
    }

    Socket * socket = server->AllocateSocket();
    if (!socket) {
        throw BaseException("TcpServer::NewConnection()", "socket is null");
    }

    socket->tcp_.data = socket;
    int err = uv_tcp_init(&server->loop_, socket->uv_tcp());
    
    if (0 == err) {
        err = uv_accept(stream, socket->uv_stream());
    }

    if (0 == err) {
        err = uv_tcp_nodelay(socket->uv_tcp(), server->GetNoDelay() ? 1 : 0);
    }

    if (0 == err) {
        if (server->GetKeepAlive() > 0) {
            err = uv_tcp_keepalive(socket->uv_tcp(), 1, server->GetKeepAlive());
        } else {
            err = uv_tcp_keepalive(socket->uv_tcp(), 0, 0);
        }
    }

    if (0 == err) {
        Buffer * address = server->Allocate();

        int address_size = (int)address->GetSize();
        err = uv_tcp_getpeername(socket->uv_tcp(), reinterpret_cast<sockaddr *>(const_cast<uint8_t *>(address->GetBuffer())), &address_size);
        if (0 == err) {
            if (address_size > address->GetSize()) {
                err = UV_EFAULT;
            } else {
                address->Use(address_size);

                socket->ClearFlag();
                socket->SetStatus(SocketOpt::S_CONNECTED);
                socket->AddFlag(SocketOpt::F_ACCEPT);
                socket->AddFlag(SocketOpt::F_READING);
                uv_read_start(socket->uv_stream(), AllocBuffer, AfterRead);

                /*
                 * Call to unqualified virtual function
                 */
                server->OnConnectionEstablished(socket, address);
            }
        }

        address->Release();
    }
    
    if (0 != err) {
        if (server->logger_) {
            server->logger_->LogError("NewConnection error: %s", uv_strerror(err));
        }

        socket->Close();
    }
}

void TcpServer::AfterClose(uv_handle_t * handle) {
    Socket * socket = static_cast<Socket *>(handle->data);
    if (!socket) {
        return;
    }

    socket->SetStatus(SocketOpt::S_DISCONNECTED);

    /*
     * Call to unqualified virtual function
     */
    socket->server_.OnConnectionClosed(socket);

    socket->Release();
}

void TcpServer::AllocBuffer(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
    Socket * socket = static_cast<Socket *>(handle->data);
    if (!socket) {
        return;
    }

    if (socket->GetStatus() != SocketOpt::S_CONNECTED) {
        return;
    }

    socket->recv_buffer_->SetupRead();
    *buf = *socket->recv_buffer_->GetUVBuffer();
}

void TcpServer::AfterRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
    Socket * socket = static_cast<Socket *>(stream->data);
    if (!socket) {
        return;
    }

    if (socket->GetStatus() != SocketOpt::S_CONNECTED) {
        return;
    }

    if (UV_EOF == nread || nread < 0) {
        if (socket->server_.logger_) {
            socket->server_.logger_->LogError("AfterRead error: %s", uv_strerror(static_cast<int>(nread)));
        }
        socket->Shutdown();
    } else {
        socket->recv_buffer_->Use(nread);

        /*
         * Call to unqualified virtual function
         */
        socket->server_.OnReadComplete(socket, socket->recv_buffer_);
    }
}

void TcpServer::AfterWrite(uv_write_t * req, int status) {
    Socket * socket = static_cast<Socket *>(req->handle->data);
    if (!socket) {
        return;
    }

    if (status < 0)
    {
        if (socket->server_.logger_) {
            socket->server_.logger_->LogError("AfterWrite error: %s", uv_strerror(status));
        }
        socket->Shutdown();
    }

    int err = status < 0 ? status : static_cast<int>(socket->send_buffer_len_);
    socket->send_buffer_len_ = 0;
    socket->RemoveFlag(SocketOpt::F_WRITING);

    /*
     * Call to unqualified virtual function
     */
    socket->server_.OnWriteComplete(socket, err);
}

TcpServer::Socket::Socket(TcpServer & server)
    : server_(server), ref_(1) {
    recv_buffer_ = server_.Allocate();
    send_buffer_ = server_.Allocate();
}

TcpServer::Socket::~Socket() {
    recv_buffer_->Release();
    send_buffer_->Release();
}

void TcpServer::Socket::AddRef() {
    ++ref_;
}

void TcpServer::Socket::Release() {
    if (--ref_ == 0) {
        server_.ReleaseSocket(this);
    }
}

bool TcpServer::Socket::SendData(const char * data, size_t length) {
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

    if (server_.IsCurrentThread()) {
        Send(data, length);
    }
    else {
        size_t avaliable = send_buffer_->GetSize() - send_buffer_->GetUsed();
        if (avaliable > length) {
            avaliable = length;
        }
        send_buffer_->AddData(data, avaliable);
        server_.AppendMessage(kSendMessage, this);
    }

    return true;
}

void TcpServer::Socket::AbortiveClose() {
    server_.AppendMessage(kKill, this);
}

void TcpServer::Socket::Shutdown() {
    if (GetStatus() == SocketOpt::S_CONNECTED) {
        return;
    }

    SetStatus(SocketOpt::S_DISCONNECTING);
    Close();
}

void TcpServer::Socket::Close() {
    if (HasFlag(SocketOpt::F_CLOSING)) {
        return;
    }

    AddFlag(SocketOpt::F_CLOSING);
    if (0 == uv_is_closing(uv_handle())) {
        uv_read_stop(uv_stream());
        uv_close(uv_handle(), TcpServer::AfterClose);
    }
}

void TcpServer::Socket::Clear() {
    Reset();
    recv_buffer_->Empty();
    send_buffer_->Empty();
}

void TcpServer::Socket::Send(const char * data, size_t length) {
    uv_buf_t buf = uv_buf_init(const_cast<char *>(data), static_cast<unsigned int>(length));
    int err = uv_write(&write_req_, uv_stream(), &buf, 1, TcpServer::AfterWrite);
    if (0 != err) {
        if (server_.logger_) {
            server_.logger_->LogError("Send error: %s", uv_strerror(err));
        }
        RemoveFlag(SocketOpt::F_WRITING);

        /*
         * Call to unqualified virtual function
         */
        server_.OnWriteComplete(this, err);
    } else {
        send_buffer_len_ = length;
    }
}