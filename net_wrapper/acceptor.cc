#include "net_wrapper/acceptor.h"

NetWrapper::CAcceptor::CContext::CContext(const std::shared_ptr<CAcceptor> & acceptor)
    : acceptor_(acceptor) {

}

NetWrapper::CAcceptor::CContext::~CContext() {

}

NetWrapper::CAcceptor::CAcceptor(CEventReactor * event_reactor)
    : CReactorHandler(event_reactor), address_(new CAddress()), open_(false) {

}

NetWrapper::CAcceptor::~CAcceptor() {
    Close();
    if (address_) {
        delete address_;
        address_ = nullptr;
    }
}

bool NetWrapper::CAcceptor::RegisterToReactor() {
    if (!socket_.GetLibuvTcp()) {
        throw BaseException(__func__, "socket_.GetLibuvTcp() == nullptr");
    }

    socket_.GetLibuvTcp()->data = new CContext(std::dynamic_pointer_cast<CAcceptor>(shared_from_this()));
    return true;
}

bool NetWrapper::CAcceptor::UnRegisterFromReactor() {
    if (!socket_.GetLibuvTcp()) {
        throw BaseException(__func__, "socket_.GetLibuvTcp() == nullptr");
    }

    CContext * context = static_cast<CContext *>(socket_.GetLibuvTcp()->data);
    delete context;
    socket_.GetLibuvTcp()->data = nullptr;
    return true;
}

bool NetWrapper::CAcceptor::Open(const CAddress & address) {
    return OpenImpl(address, ADDRESSFAMILY_IPV4);
}

bool NetWrapper::CAcceptor::Open(const char * ip, uint16_t port) {
    return Open(CAddress(ip, port));
}

bool NetWrapper::CAcceptor::Open6(const CAddress & address) {
    return OpenImpl(address, ADDRESSFAMILY_IPV6);
}

bool NetWrapper::CAcceptor::Open6(const char * ip, uint16_t port) {
    return Open6(CAddress(ip, port));
}

bool NetWrapper::CAcceptor::OpenImpl(const CAddress & address, EAddressFamily family) {
    if (open_) {
        return false;
    }

    *address_ = address;
    socket_.Open(GetEventReactor());
    int err;
    if (ADDRESSFAMILY_IPV4 == family) {
        err = socket_.Bind(*address_);
    } else {
        err = socket_.Bind6(*address_);
    }
    if (0 != err) {
        std::stringstream ss;
        ss << "bind failed error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }

    err = uv_listen(reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), 128, Accept);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_listen error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }

    GetEventReactor()->Register(this);
    open_ = true;
    return true;
}

void NetWrapper::CAcceptor::Close(void) {
    if (open_) {
        open_ = false;
        GetEventReactor()->UnRegister(this);
        socket_.Close();
    }
}

void NetWrapper::CAcceptor::AcceptEventPipe(CEventPipe * event_pipe, uv_tcp_t * tcp) {
    if (!tcp) {
        throw BaseException(__func__, "tcp == nullptr");
    }
    if (!event_pipe) {
        throw BaseException(__func__, "event_pipe == nullptr");
    }
    event_pipe->GetSocket()->SetLibuvTcp(tcp);
    event_pipe->GetSocket()->SetNoDelay();
}

bool NetWrapper::CAcceptor::ActivateEventPipe(CEventPipe * event_pipe) {
    event_pipe->SetEventReactor(GetEventReactor());
    return event_pipe->Open();
}

void NetWrapper::CAcceptor::Accept() {
    uv_tcp_t * tcp = CSocket::CreateLibuvTcp(GetEventReactor()->GetLibuvLoop());
    int err = uv_accept(reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), reinterpret_cast<uv_stream_t *>(tcp));
    if (0 != err) {
        OutputMsg(Logger::Error, "uv_accept error[%s]", uv_strerror(err));
        uv_close(reinterpret_cast<uv_handle_t *>(tcp), CSocket::LibuvCloseCb);
        return;
    }
    
    CEventPipe * pipe = nullptr;
    MakeEventPipe(pipe);
    AcceptEventPipe(pipe, tcp);
    if (ActivateEventPipe(pipe)) {
        OnAccepted(pipe);
        pipe->OnConnected();
    }
}

void NetWrapper::CAcceptor::Accept(uv_stream_t * stream, int status) {
    if (0 != status) {
        OutputMsg(Logger::Error, "accept failed error[%s]", uv_strerror(status));
    } else {
        CContext * context = static_cast<CContext *>(stream->data);
        if (context) {
            std::shared_ptr<CAcceptor> acceptor = context->acceptor_.lock();
            if (acceptor) {
                acceptor->Accept();
            }
        }
    }
}