#include "net_wrapper/event_pipe.h"

NetWrapper::CEventPipe::CEventPipe(uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CReactorHandler(nullptr), connect_req_(nullptr), delay_shutdown_timer_(nullptr)
    , connect_state_(SocketOpt::S_DISCONNECTED), max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size)
    , reg_timer_(false), shutdown_(false), called_on_disconnect_(false), called_on_disconnected_(false) {
    
}

NetWrapper::CEventPipe::~CEventPipe() {
    DelTimer();
    ShutdownImmediately();
    if (delay_shutdown_timer_) {
        uv_close(reinterpret_cast<uv_handle_t *>(delay_shutdown_timer_), LibuvTimerCloseCb);
        delay_shutdown_timer_ = nullptr;
    }
}

bool NetWrapper::CEventPipe::RegisterToReactor() {
    throw std::logic_error("The method or operation is not implemented.");
}

bool NetWrapper::CEventPipe::UnRegisterFromReactor() {
    throw std::logic_error("The method or operation is not implemented.");
}

void NetWrapper::CEventPipe::HandleClose4EOF(EPipeDisconnectReason reason, bool remote) {
    if (connect_state_ == SocketOpt::S_CONNECTED || connect_state_ == SocketOpt::S_DISCONNECTING) {
        connect_state_ = SocketOpt::S_DISCONNECTING;
        if (shutdown_) {
            if (reg_timer_) {
                DelTimer();
                ShutdownImmediately();
                CallOnDisconnected(false);
            } else {
                socket_.SetShutdownRead(true);
            }
        } else {
            if (called_on_disconnect_) {
                throw BaseException(__func__, "called_on_disconnect_ == true");
            }
            OnDisconnect(reason, remote);
            called_on_disconnect_ = true;
            if (output_->GetLength() > 0) {
                socket_.SetShutdownRead(true);
            } else {
                ShutdownImmediately();
                CallOnDisconnected(true);
            }
        }
    }
}

void NetWrapper::CEventPipe::HandleClose4Error(EPipeDisconnectReason reason, bool remote) {

}

int NetWrapper::CEventPipe::Write(const uint8_t * data, size_t len) {

}

uint8_t * NetWrapper::CEventPipe::ReverseGetOutputData(size_t len) {

}

uint8_t * NetWrapper::CEventPipe::GetRecvData() const {

}

size_t NetWrapper::CEventPipe::GetRecvDataSize() const {

}

void NetWrapper::CEventPipe::PopRecvData(size_t len) {

}

void NetWrapper::CEventPipe::ClearInBuf() {

}

void NetWrapper::CEventPipe::ClearOutBuf() {

}

bool NetWrapper::CEventPipe::Open() {
    return GetEventReactor()->Register(this);
}

void NetWrapper::CEventPipe::Shutdown(bool now) {
    if (connect_state_ == SocketOpt::S_CONNECTING) {
        ShutdownImmediately();
        shutdown_;
    }
    if (m_ConnectState == eConnecting) {
        ShutdownImmediately();
        m_bShutdown = true;
    }
    else if (m_ConnectState > eConnecting) {
        if (m_ConnectState > eDisconnected) {
            throw ArkCommon::CArkException("Assertion failure of expresion 'false'", __FILE__, __DATE__, __TIME__, __LINE__, __FUNCTION__, true, true);
        }
    }
    else {
        if (m_ConnectState != eConnected) {
            throw ArkCommon::CArkException("Assertion failure of expresion 'false'", __FILE__, __DATE__, __TIME__, __LINE__, __FUNCTION__, true, true);
        }
        m_ConnectState = eDisconnecting;
        m_bShutdown = true;
        if (m_ArkSocket.IsShutdownRead()) {
            throw ArkCommon::CArkException("Assertion failure of expresion '!m_ArkSocket.IsShutdownRead()'", __FILE__, __DATE__, __TIME__, __LINE__, __FUNCTION__, true, true);
        }
        if (m_pStBufEv->output->off && !now) {
            AddTick(30);
        }
        else {
            m_ArkSocket.ShutdownWrite();
            AddTick(10);
        }
    }
}

void NetWrapper::CEventPipe::ShutdownImmediately() {
    if (connect_state_ == SocketOpt::S_CONNECTING) {
        if (connect_req_) {
            connect_req_ = nullptr;
        }
        socket_.Shutdown();
        connect_state_ = SocketOpt::S_DISCONNECTED;
        OnConnectFailed(CONNFAILEDREASON_ECANCELED);
    } else if (connect_state_ == SocketOpt::S_CONNECTED) {
        UnRegisterFromReactor();
        socket_.Shutdown();
    }
    else if (m_ConnectState <= eConnecting) {
        if (m_ConnectState != eConnected) {
            throw ArkCommon::CArkException("Assertion failure of expresion 'false'", __FILE__, __DATE__, __TIME__, __LINE__, __FUNCTION__, true, true);
        }
        UnRegisterFromReactor();
        m_ArkSocket.Shutdown();
    }
    else if (m_ConnectState == eDisconnecting) {
        UnRegisterFromReactor();
        m_ArkSocket.Shutdown();
    }
    else if (m_ConnectState != eDisconnected) {
        throw ArkCommon::CArkException("Assertion failure of expresion 'false'", __FILE__, __DATE__, __TIME__, __LINE__, __FUNCTION__, true, true);
    }
}
