#include "net_wrapper/acceptor.h"

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
    return true;
}

bool NetWrapper::CAcceptor::UnRegisterFromReactor() {
    return true;
}

bool NetWrapper::CAcceptor::Open(const char * ip, uint16_t port) {
    return Open(CAddress(ip, port));
}

bool NetWrapper::CAcceptor::Open(const CAddress & address) {
    if (open_) {
        return false;
    }

    *address_ = address;
    socket_.Open(GetEventReactor());
    if (0 != socket_.Bind(*address_)) {
        throw BaseException(__func__, "socket_.Bind(*address_) != 0");
    }

    int err = uv_listen(reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), 128, Accept);
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

void NetWrapper::CAcceptor::AcceptEventPipe(CEventPipe * event_pipe) {

}

bool NetWrapper::CAcceptor::ActivateEventPipe(CEventPipe * event_pipe) {

}

void NetWrapper::CAcceptor::Accept(uv_stream_t * stream, int status) {

}

