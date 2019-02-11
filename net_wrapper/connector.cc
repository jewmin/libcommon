#include "net_wrapper/connector.h"

NetWrapper::CConnector::CContext::CContext(CConnector * connector, CEventPipe * event_pipe)
    : connector_(connector), event_pipe_(event_pipe) {

}

NetWrapper::CConnector::CContext::~CContext() {

}

NetWrapper::CConnector::CConnector(CEventReactor * event_reactor)
    : CReactorHandler(event_reactor) {

}

NetWrapper::CConnector::~CConnector() {

}

bool NetWrapper::CConnector::RegisterToReactor() {
    throw BaseException(__func__, "The method or operation is not implemented.");
}

bool NetWrapper::CConnector::UnRegisterFromReactor() {
    throw BaseException(__func__, "The method or operation is not implemented.");
}

void NetWrapper::CConnector::Connect(CEventPipe * event_pipe, const CAddress & address, int & ret) {
    ConnectImpl(event_pipe, address, ret, ADDRESSFAMILY_IPV4);
}

void NetWrapper::CConnector::Connect(CEventPipe * event_pipe, const char * ip, uint16_t port, int & ret) {
    Connect(event_pipe, CAddress(ip, port), ret);
}

void NetWrapper::CConnector::Connect6(CEventPipe * event_pipe, const CAddress & address, int & ret) {
    ConnectImpl(event_pipe, address, ret, ADDRESSFAMILY_IPV6);
}

void NetWrapper::CConnector::Connect6(CEventPipe * event_pipe, const char * ip, uint16_t port, int & ret) {
    Connect(event_pipe, CAddress(ip, port), ret);
}

void NetWrapper::CConnector::ConnectImpl(CEventPipe * event_pipe, const CAddress & address, int & ret, EAddressFamily family) {
    if (!event_pipe) {
        throw BaseException(__func__, "event_pipe == nullptr");
    }

    if (!event_pipe->GetSocket()->GetLibuvTcp()) {
        event_pipe->GetSocket()->Open(GetEventReactor());
    }
    event_pipe->GetSocket()->SetNoDelay();

    union {
        struct sockaddr addr;
        struct sockaddr_in addr4;
        struct sockaddr_in6 addr6;
    } s;
    if (ADDRESSFAMILY_IPV4 == family) {
        CSocket::Address2sockaddr_in(s.addr4, address);
    } else {
        CSocket::Address2sockaddr_in6(s.addr6, address);
    }

    event_pipe->SetConnectReq(new uv_connect_t());
    CContext * context = new CContext(this, event_pipe);
    event_pipe->GetConnectReq()->data = context;
    int err = uv_tcp_connect(event_pipe->GetConnectReq(), event_pipe->GetSocket()->GetLibuvTcp(), &s.addr, LibuvCb);
    if (0 != err) {
        delete event_pipe->GetConnectReq();
        event_pipe->SetConnectReq(nullptr);
        delete context;

        ret = 2;
        EPipeConnFailedReason reason;
        switch (err) {
            case UV_EADDRNOTAVAIL:
            reason = CONNFAILEDREASON_EADDRNOTAVAIL;
            break;

            case UV_EADDRINUSE:
            reason = CONNFAILEDREASON_EADDRINUSE;
            break;

            case UV_ENETUNREACH:
            case UV_EHOSTUNREACH:
            reason = CONNFAILEDREASON_ENETUNREACH;
            break;

            case UV_ECONNRESET:
            reason = CONNFAILEDREASON_ECONNRESET;
            break;

            case UV_ETIMEDOUT:
            reason = CONNFAILEDREASON_ETIMEDOUT;
            break;

            case UV_ENOTCONN:
            case UV_ECONNREFUSED:
            reason = CONNFAILEDREASON_ECONNREFUSED;
            break;

            case UV_ECANCELED:
            reason = CONNFAILEDREASON_ECANCELED;
            break;

            default:
            OutputMsg(Logger::Error, "uv_tcp_connect error[%s]", uv_strerror(err));
            reason = CONNFAILEDREASON_UNKNOWN;
            break;
        }
        event_pipe->GetSocket()->Close();
        event_pipe->OnConnectFailed(reason);
    } else {
        ret = 1;
        event_pipe->SetConnectState(SocketOpt::S_CONNECTING);
    }
}

bool NetWrapper::CConnector::ActivateEventPipe(CEventPipe * event_pipe) {
    event_pipe->SetEventReactor(GetEventReactor());
    return event_pipe->Open();
}

void NetWrapper::CConnector::OnOneConnectSuccess(CEventPipe * event_pipe) {
    if (!ActivateEventPipe(event_pipe)) {
        throw BaseException(__func__, "ActivateEventPipe(event_pipe) == false");
    }
    event_pipe->OnConnected();
}

void NetWrapper::CConnector::LibuvCb(uv_connect_t * req, int status) {
    CContext * context = static_cast<CContext *>(req->data);
    CConnector * connector = context->connector_;
    CEventPipe * event_pipe = context->event_pipe_;
    delete event_pipe->GetConnectReq();
    event_pipe->SetConnectReq(nullptr);
    delete context;

    if (0 != status) {
        EPipeConnFailedReason reason;
        switch (status) {
            case UV_EADDRNOTAVAIL:
            reason = CONNFAILEDREASON_EADDRNOTAVAIL;
            break;

            case UV_EADDRINUSE:
            reason = CONNFAILEDREASON_EADDRINUSE;
            break;

            case UV_ENETUNREACH:
            case UV_EHOSTUNREACH:
            reason = CONNFAILEDREASON_ENETUNREACH;
            break;

            case UV_ECONNRESET:
            reason = CONNFAILEDREASON_ECONNRESET;
            break;

            case UV_ETIMEDOUT:
            reason = CONNFAILEDREASON_ETIMEDOUT;
            break;

            case UV_ENOTCONN:
            case UV_ECONNREFUSED:
            reason = CONNFAILEDREASON_ECONNREFUSED;
            break;

            case UV_ECANCELED:
            reason = CONNFAILEDREASON_ECANCELED;
            break;

            default:
            OutputMsg(Logger::Error, "connect failed error [%s]", uv_strerror(status));
            reason = CONNFAILEDREASON_UNKNOWN;
            break;
        }
        event_pipe->GetSocket()->Close();
        event_pipe->SetConnectState(SocketOpt::S_DISCONNECTED);
        event_pipe->OnConnectFailed(reason);
    } else {
        connector->OnOneConnectSuccess(event_pipe);
    }
}