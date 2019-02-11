#include "net_wrapper/event_pipe.h"

NetWrapper::CEventPipe::CEventPipe(uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CReactorHandler(nullptr), connect_req_(nullptr), delay_shutdown_timer_(nullptr)
    , input_(nullptr), output_(nullptr), buffer_(nullptr), connect_state_(SocketOpt::S_DISCONNECTED)
    , max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size)
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
    if (SocketOpt::S_CONNECTING != connect_state_ && SocketOpt::S_DISCONNECTED != connect_state_) {
        return false;
    }

    if (!socket_.GetLibuvTcp()) {
        throw BaseException(__func__, "socket_.GetLibuvTcp() == nullptr");
    }

    if (!GetEventReactor()) {
        throw BaseException(__func__, "GetEventReactor() == nullptr");
    }

    if (!delay_shutdown_timer_) {
        delay_shutdown_timer_ = new uv_timer_t();
        delay_shutdown_timer_->data = this;
        int err = uv_timer_init(GetEventReactor()->GetLibuvLoop(), delay_shutdown_timer_);
        if (0 != err) {
            std::stringstream ss;
            ss << "uv_timer_init error[" << uv_strerror(err) << "]";
            throw BaseException(__func__, ss.str());
        }
    }

    if (input_ || output_) {
        throw BaseException(__func__, "input_ != nullptr || output_ != nullptr");
    }

    input_ = new Packet();
    output_ = new Packet();
    if (!input_ || !output_) {
        throw BaseException(__func__, "input_ == nullptr || output_ == nullptr");
    }

    // input_->Reserve(max_in_buffer_size_);
    // output_->Reserve(max_out_buffer_size_);
    output_->SetAllocSize(1024);
    int err = uv_read_start(reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), LibuvAllocCb, LibuvReadCb);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_read_start error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }

    socket_.GetLibuvTcp()->data = this;
    SetConnectState(SocketOpt::S_CONNECTED);
    return true;
}

bool NetWrapper::CEventPipe::UnRegisterFromReactor() {
    if (SocketOpt::S_CONNECTED != connect_state_ && SocketOpt::S_DISCONNECTING != connect_state_) {
        return false;
    }

    SetConnectState(SocketOpt::S_DISCONNECTED);
    socket_.GetLibuvTcp()->data = nullptr;

    if (!input_ || !output_) {
        OutputMsg(Logger::Warn, "input_ == nullptr || output_ == nullptr");
    }

    if (input_) {
        delete input_;
        input_ = nullptr;
    }

    if (output_) {
        delete output_;
        output_ = nullptr;
    }
    
    return true;
}

void NetWrapper::CEventPipe::HandleClose4EOF(EPipeDisconnectReason reason, bool remote) {
    if (SocketOpt::S_CONNECTED != connect_state_ && SocketOpt::S_DISCONNECTING != connect_state_) {
        SetConnectState(SocketOpt::S_DISCONNECTING);
        if (shutdown_) {
            if (reg_timer_) {
                DelTimer();
                ShutdownImmediately();
                CallOnDisconnected(false);
            } else {
                socket_.ShutdownRead();
            }
        } else {
            if (called_on_disconnect_) {
                throw BaseException(__func__, "called_on_disconnect_ == true");
            }
            OnDisconnect(reason, remote);
            called_on_disconnect_ = true;
            if (output_->GetLength() > 0) {
                socket_.ShutdownRead();
            } else {
                ShutdownImmediately();
                CallOnDisconnected(true);
            }
        }
    }
}

void NetWrapper::CEventPipe::HandleClose4Error(EPipeDisconnectReason reason, bool remote) {
    if (SocketOpt::S_CONNECTED != connect_state_ && SocketOpt::S_DISCONNECTING != connect_state_) {
        SetConnectState(SocketOpt::S_DISCONNECTING);
        if (shutdown_) {
            DelTimer();
            ShutdownImmediately();
            CallOnDisconnected(false);
        } else {
            if (called_on_disconnect_) {
                throw BaseException(__func__, "called_on_disconnect_ == true");
            }
            OnDisconnect(reason, remote);
            called_on_disconnect_ = true;
            ShutdownImmediately();
            CallOnDisconnected(true);
        }
    }
}

int NetWrapper::CEventPipe::Write(const uint8_t * data, size_t len) {
    if (SocketOpt::S_CONNECTED != connect_state_) {
        return -3;
    }

    if (max_out_buffer_size_ > 0 && output_->GetLength() > max_out_buffer_size_) {
        OutputMsg(Logger::Warn, "---当前写缓冲区 数据大小/需要写入大小/总大小/上限:%zd / %zd / %zd / %u----", len, output_->GetLength(), output_->GetSize(), max_out_buffer_size_);
        return -2;
    }

    output_->WriteBinary(data, len);
    if (0 == output_size_) {
        uv_write_t * req = new uv_write_t();
        uv_buf_t buf = uv_buf_init(reinterpret_cast<char *>(output_->GetMemoryPtr()), output_->GetLength());
        int err = uv_write(req, reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), &buf, 1, nullptr);
        if (0 != err) {
            delete req;
            HandleClose4Error(DISCONNECTREASON_ERROR, true);
        }
        return err;
    } else {
        return 0;
    }
}

void NetWrapper::CEventPipe::CancelWrite(size_t len) {
    if (output_->GetLength() < len) {
        throw BaseException(__func__, "output_->GetLength() < len");
    }
    output_->SetLength(output_->GetLength() - len);
}

uint8_t * NetWrapper::CEventPipe::ReverseGetOutputData(size_t len) {
    if (output_->GetLength() < len) {
        throw BaseException(__func__, "output_->GetLength() < len");
    }
    return output_->GetPositionPtr(output_->GetLength() - len);
}

uint8_t * NetWrapper::CEventPipe::GetRecvData() const {
    if (!input_) {
        throw BaseException(__func__, "input_ == nullptr");
    }
    return input_->GetOffsetPtr();
}

size_t NetWrapper::CEventPipe::GetRecvDataSize() const {
    if (!input_) {
        throw BaseException(__func__, "input_ == nullptr");
    }
    return input_->GetReadableLength();
}

void NetWrapper::CEventPipe::PopRecvData(size_t len) {
    if (SocketOpt::S_CONNECTED != connect_state_ && SocketOpt::S_DISCONNECTING != connect_state_) {
        OutputMsg(Logger::Error, "---在连接状态为非 S_CONNECTED / S_DISCONNECTING 上调用了 PopRecvData(size_t)!---");
    } else if (!input_) {
        throw BaseException(__func__, "input_ == nullptr");
    } else if (input_->GetReadableLength() < len) {
        throw BaseException(__func__, "input_->GetReadableLength() < len");
    } else if (len > 0) {
        if (len >= input_->GetReadableLength()) {
            input_->SetLength(0);
        } else {
            input_->AdjustOffset(len);
        }
    }
}

void NetWrapper::CEventPipe::ClearInBuf() {
    input_->SetLength(0);
}

void NetWrapper::CEventPipe::ClearOutBuf() {
    output_->SetLength(0);
}

bool NetWrapper::CEventPipe::Open() {
    return GetEventReactor()->Register(this);
}

void NetWrapper::CEventPipe::Shutdown(bool now) {
    if (SocketOpt::S_CONNECTING == connect_state_) {
        ShutdownImmediately();
        shutdown_ = true;
    } else if (SocketOpt::S_CONNECTED == connect_state_) {
        SetConnectState(SocketOpt::S_DISCONNECTING);
        shutdown_ = true;
        if (socket_.IsShutdownRead()) {
            throw BaseException(__func__, "socket_.IsShutdownRead() == true");
        }
        if (output_->GetLength() > 0 && !now) {
            AddTimer(30);
        } else {
            socket_.ShutdownWrite();
            AddTimer(10);
        }
    }
}

void NetWrapper::CEventPipe::ShutdownImmediately() {
    if (SocketOpt::S_CONNECTING == connect_state_) {
        socket_.Close();
        SetConnectState(SocketOpt::S_DISCONNECTED);
    } else if (SocketOpt::S_CONNECTED == connect_state_ || SocketOpt::S_DISCONNECTING == connect_state_) {
        socket_.Shutdown();
        GetEventReactor()->UnRegister(this);
    } else if (SocketOpt::S_DISCONNECTED == connect_state_) {
        throw BaseException(__func__, "connect_state_ == SocketOpt::S_DISCONNECTED");
    }
}

void NetWrapper::CEventPipe::CallOnDisconnected(bool remote) {
    if (called_on_disconnected_) {
        throw BaseException(__func__, "called_on_disconnected_ == true");
    }
    OnDisconnected(DISCONNECTREASON_EOF, remote);
    called_on_disconnected_ = true;
}

void NetWrapper::CEventPipe::SetMaxOutBufferSize(uint32_t size) {
    if (0 == size) {
        throw BaseException(__func__, "size == 0");
    }

    max_out_buffer_size_ = size;
    // if (SocketOpt::S_CONNECTED == connect_state_) {
    //     if (!output_) {
    //         throw BaseException(__func__, "output_ == nullptr");
    //     }
    //     output_->Reserve(max_out_buffer_size_);
    // }
}

void NetWrapper::CEventPipe::SetMaxInBufferSize(uint32_t size) {
    if (0 == size) {
        throw BaseException(__func__, "size == 0");
    }

    max_in_buffer_size_ = size;
    // if (SocketOpt::S_CONNECTED == connect_state_) {
    //     if (!input_) {
    //         throw BaseException(__func__, "input_ == nullptr");
    //     }
    //     input_->Reserve(max_in_buffer_size_);
    // }
}

size_t NetWrapper::CEventPipe::GetInBufferSize() {
    if (input_) {
        return input_->GetSize();
    } else {
        return 0;
    }
}

size_t NetWrapper::CEventPipe::GetOutBufferSize() {
    if (output_) {
        return output_->GetSize();
    } else {
        return 0;
    }
}

size_t NetWrapper::CEventPipe::GetOutBufferUsedSize() {
    if (output_) {
        return output_->GetLength();
    } else {
        return 0;
    }
}

void NetWrapper::CEventPipe::AddTimer(uint32_t interval) {
    if (reg_timer_) {
        throw BaseException(__func__, "reg_timer_ == true");
    }

    int err = uv_timer_start(delay_shutdown_timer_, LibuvTimerCb, interval * 1000, 0);
    if (0 != err) {
        CAddress address;
        GetSocket()->GetRemoteAddress(address);
        OutputMsg(Logger::Error, "AddTimer error:%s, IP:%s, second:%u", uv_strerror(err), address.ToString().c_str(), interval);
    } else {
        CAddress address;
        GetSocket()->GetRemoteAddress(address);
        OutputMsg(Logger::Info, "AddTimer success, IP:%s, second:%u", address.ToString().c_str(), interval);
    }
    reg_timer_ = true;
}

void NetWrapper::CEventPipe::DelTimer() {
    if (reg_timer_) {
        uv_timer_stop(delay_shutdown_timer_);
        reg_timer_ = false;
    }
}

void NetWrapper::CEventPipe::DumpOutBuf() {
    Dump(output_->GetMemoryPtr(), output_->GetLength());
}

void NetWrapper::CEventPipe::DumpInBuf() {
    Dump(input_->GetOffsetPtr(), input_->GetReadableLength());
}

void NetWrapper::CEventPipe::Dump(uint8_t * data, size_t len) {

}

void NetWrapper::CEventPipe::LibuvTimerCb(uv_timer_t * handle) {
    CEventPipe * event_pipe = static_cast<CEventPipe *>(handle->data);
    CAddress address;
    event_pipe->GetSocket()->GetRemoteAddress(address);
    OutputMsg(Logger::Info, "LibuvTimerCb, IP:%s", address.ToString().c_str());
    event_pipe->ShutdownImmediately();
    event_pipe->CallOnDisconnected(false);
    event_pipe->reg_timer_ = false;
}

void NetWrapper::CEventPipe::LibuvTimerCloseCb(uv_handle_t * handle) {
    delete handle;
}

void NetWrapper::CEventPipe::LibuvAllocCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
    CEventPipe * event_pipe = static_cast<CEventPipe *>(handle->data);
    buf->base = reinterpret_cast<char *>(event_pipe->input_->GetOffsetPtr());
    buf->len = event_pipe->input_->GetWritableLength();
}

void NetWrapper::CEventPipe::LibuvReadCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
    CEventPipe * event_pipe = static_cast<CEventPipe *>(handle->data);
    if (pPipe->m_ConnectState != eDisconnecting) {
        pPipe->OnNewDataReceived();
    }
}
