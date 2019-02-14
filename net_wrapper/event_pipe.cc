#include "net_wrapper/event_pipe.h"

NetWrapper::CEventPipe::CContext::CContext(const std::shared_ptr<CEventPipe> & event_pipe)
    : event_pipe_(event_pipe) {

}

NetWrapper::CEventPipe::CContext::~CContext() {

}

NetWrapper::CEventPipe::WriteRequest::WriteRequest(const std::shared_ptr<CEventPipe> & event_pipe, const char * data, size_t size)
    : event_pipe_(event_pipe), storage_(data, data + size), buf_(uv_buf_init(&storage_[0], static_cast<unsigned int>(storage_.size()))) {
    req_.data = this;
}

NetWrapper::CEventPipe::WriteRequest::~WriteRequest() {

}

NetWrapper::CEventPipe::CIOBuffer::CIOBuffer(CEventPipe * event_pipe)
    : event_pipe_(event_pipe), input_(new Packet()), output_(new Packet()), request_(nullptr) {
    if (!input_ || !output_) {
        throw BaseException(__func__, "input_ == nullptr || output_ == nullptr");
    }
    output_->SetAllocSize(1024);
}

NetWrapper::CEventPipe::CIOBuffer::~CIOBuffer() {
    delete input_;
    delete output_;
}

void NetWrapper::CEventPipe::CIOBuffer::InitWriteRequest() {
    request_ = new WriteRequest(std::dynamic_pointer_cast<CEventPipe>(event_pipe_->shared_from_this()), reinterpret_cast<char *>(output_->GetMemoryPtr()), output_->GetLength());
    output_->SetLength(0);
}

NetWrapper::CEventPipe::CEventPipe(uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CReactorHandler(nullptr), delay_shutdown_timer_(nullptr), buffer_(nullptr), connect_state_(SocketOpt::S_DISCONNECTED)
    , max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size)
    , reg_timer_(false), shutdown_(false), called_on_disconnect_(false), called_on_disconnected_(false) {
    
}

NetWrapper::CEventPipe::~CEventPipe() {
    DelTimer();
    ShutdownImmediately();
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

    if (delay_shutdown_timer_) {
        throw BaseException(__func__, "delay_shutdown_timer_ != nullptr");
    }

    delay_shutdown_timer_ = new uv_timer_t();
    delay_shutdown_timer_->data = new CContext(std::dynamic_pointer_cast<CEventPipe>(shared_from_this()));
    int err = uv_timer_init(GetEventReactor()->GetLibuvLoop(), delay_shutdown_timer_);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_timer_init error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }

    if (buffer_) {
        throw BaseException(__func__, "buffer_ != nullptr");
    }

    buffer_ = new CIOBuffer(this);
    if (!buffer_) {
        throw BaseException(__func__, "buffer_ == nullptr");
    }
    
    err = uv_read_start(reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), LibuvAllocCb, LibuvReadCb);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_read_start error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }

    socket_.GetLibuvTcp()->data = new CContext(std::dynamic_pointer_cast<CEventPipe>(shared_from_this()));
    SetConnectState(SocketOpt::S_CONNECTED);
    return true;
}

bool NetWrapper::CEventPipe::UnRegisterFromReactor() {
    if (SocketOpt::S_CONNECTED != connect_state_ && SocketOpt::S_DISCONNECTING != connect_state_) {
        return false;
    }

    SetConnectState(SocketOpt::S_DISCONNECTED);
    CContext * context = static_cast<CContext *>(socket_.GetLibuvTcp()->data);
    delete context;
    socket_.GetLibuvTcp()->data = nullptr;

    if (!buffer_) {
        OutputMsg(Logger::Warn, "buffer_ == nullptr");
    } else {
        delete buffer_;
        buffer_ = nullptr;
    }

    if (!delay_shutdown_timer_) {
        OutputMsg(Logger::Warn, "delay_shutdown_timer_ == nullptr");
    } else {
        CContext * context = static_cast<CContext *>(delay_shutdown_timer_->data);
        delete context;
        delay_shutdown_timer_->data = nullptr;
        uv_close(reinterpret_cast<uv_handle_t *>(delay_shutdown_timer_), LibuvTimerCloseCb);
        delay_shutdown_timer_ = nullptr;
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
            if (buffer_->output_->GetLength() > 0) {
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

    if (max_out_buffer_size_ > 0 && buffer_->output_->GetLength() > max_out_buffer_size_) {
        OutputMsg(Logger::Warn, "---当前写缓冲区 数据大小/需要写入大小/总大小/上限:%zd / %zd / %zd / %u----", len, buffer_->output_->GetLength(), buffer_->output_->GetSize(), max_out_buffer_size_);
        return -2;
    }

    buffer_->output_->WriteBinary(data, len);
    if (!buffer_->request_) {
        buffer_->InitWriteRequest();
        int err = uv_write(buffer_->GetLibuvWriteReq(), reinterpret_cast<uv_stream_t *>(socket_.GetLibuvTcp()), buffer_->GetLibuvBuf(), 1, LibuvWriteCb);
        if (0 != err) {
            delete buffer_->request_;
            buffer_->request_ = nullptr;
            HandleError(err);
        }
        return err;
    } else {
        return 0;
    }
}

void NetWrapper::CEventPipe::CancelWrite(size_t len) {
    if (buffer_->output_->GetLength() < len) {
        throw BaseException(__func__, "buffer_->output_->GetLength() < len");
    }
    buffer_->output_->SetLength(buffer_->output_->GetLength() - len);
}

uint8_t * NetWrapper::CEventPipe::ReverseGetOutputData(size_t len) {
    if (buffer_->output_->GetLength() < len) {
        throw BaseException(__func__, "buffer_->output_->GetLength() < len");
    }
    return buffer_->output_->GetPositionPtr(buffer_->output_->GetLength() - len);
}

uint8_t * NetWrapper::CEventPipe::GetRecvData() const {
    if (!buffer_->input_) {
        throw BaseException(__func__, "buffer_->input_ == nullptr");
    }
    return buffer_->input_->GetOffsetPtr();
}

size_t NetWrapper::CEventPipe::GetRecvDataSize() const {
    if (!buffer_->input_) {
        throw BaseException(__func__, "buffer_->input_ == nullptr");
    }
    return buffer_->input_->GetReadableLength();
}

void NetWrapper::CEventPipe::PopRecvData(size_t len) {
    if (SocketOpt::S_CONNECTED != connect_state_ && SocketOpt::S_DISCONNECTING != connect_state_) {
        OutputMsg(Logger::Error, "---在连接状态为非 S_CONNECTED / S_DISCONNECTING 上调用了 PopRecvData(size_t)!---");
    } else if (!buffer_->input_) {
        throw BaseException(__func__, "buffer_->input_ == nullptr");
    } else if (buffer_->input_->GetReadableLength() < len) {
        throw BaseException(__func__, "buffer_->input_->GetReadableLength() < len");
    } else if (len > 0) {
        if (len >= buffer_->input_->GetReadableLength()) {
            buffer_->input_->SetLength(0);
        } else {
            buffer_->input_->AdjustOffset(len);
        }
    }
}

void NetWrapper::CEventPipe::ClearInBuf() {
    buffer_->input_->SetLength(0);
}

void NetWrapper::CEventPipe::ClearOutBuf() {
    buffer_->output_->SetLength(0);
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
        if (buffer_->output_->GetLength() > 0 && !now) {
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

void NetWrapper::CEventPipe::HandleError(int error) {
    OnError(error);
    if (UV_EOF == error) {
        HandleClose4EOF(DISCONNECTREASON_EOF, true);
    } else {
        if (UV_EPIPE == error) {
            OutputMsg(Logger::Warn, "网络出错，状态为(UV_EPIPE 连接已经完全断开,还继续写数据?) NO:%d", error);
            HandleClose4Error(DISCONNECTREASON_ERROR, true);
        } else if (UV_ETIMEDOUT == error || UV_ECONNABORTED == error) {
            OutputMsg(Logger::Warn, "网络出错，状态为(UV_ETIMEDOUT|ECONNABORTED) NO:%d", error);
            HandleClose4Error(DISCONNECTREASON_ERROR, true);
        } else if (UV_EHOSTUNREACH == error || UV_ECONNRESET == error) {
            HandleClose4Error(DISCONNECTREASON_ERROR, true);
        } else if (UV_EINTR == error || UV_EAGAIN == error) {
            std::stringstream ss;
            ss << "recv failed with error code (EWOULDBLOCK|WSA_OPERATION_ABORTED) NO:" << error;
            throw BaseException(__func__, ss.str());
        } else if (0 != error) {
            OutputMsg(Logger::Warn, "网络出错，状态为(default) NO:%d", error);
            HandleClose4Error(DISCONNECTREASON_ERROR, true);
        }
    }
}

void NetWrapper::CEventPipe::SetMaxOutBufferSize(uint32_t size) {
    if (0 == size) {
        throw BaseException(__func__, "size == 0");
    }

    max_out_buffer_size_ = size;
    // if (SocketOpt::S_CONNECTED == connect_state_) {
    //     if (!buffer_->output_) {
    //         throw BaseException(__func__, "buffer_->output_ == nullptr");
    //     }
    //     buffer_->output_->Reserve(max_out_buffer_size_);
    // }
}

void NetWrapper::CEventPipe::SetMaxInBufferSize(uint32_t size) {
    if (0 == size) {
        throw BaseException(__func__, "size == 0");
    }

    max_in_buffer_size_ = size;
    // if (SocketOpt::S_CONNECTED == connect_state_) {
    //     if (!buffer_->input_) {
    //         throw BaseException(__func__, "buffer_->input_ == nullptr");
    //     }
    //     buffer_->input_->Reserve(max_in_buffer_size_);
    // }
}

size_t NetWrapper::CEventPipe::GetInBufferSize() {
    if (buffer_->input_) {
        return buffer_->input_->GetSize();
    } else {
        return 0;
    }
}

size_t NetWrapper::CEventPipe::GetOutBufferSize() {
    if (buffer_->output_) {
        return buffer_->output_->GetSize();
    } else {
        return 0;
    }
}

size_t NetWrapper::CEventPipe::GetOutBufferUsedSize() {
    if (buffer_->output_) {
        return buffer_->output_->GetLength();
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
    Dump(buffer_->output_->GetMemoryPtr(), buffer_->output_->GetLength());
}

void NetWrapper::CEventPipe::DumpInBuf() {
    Dump(buffer_->input_->GetOffsetPtr(), buffer_->input_->GetReadableLength());
}

void NetWrapper::CEventPipe::Dump(uint8_t * data, size_t len) {
    uint32_t line = 1;
    uint32_t count_pos = 0;
    uint32_t length = static_cast<uint32_t>(len);
    uint32_t count = 0;
    printf("Packet: PacketSize = %u\n", length);
    printf("|------------------------------------------------|----------------|\n");
    printf("|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|\n");
    printf("|------------------------------------------------|----------------|\n");
    if (length > 0) {
        printf("|");
        for (count = 0; count < length; ++count) {
            if (16 == count_pos) {
                count_pos = 0;
                printf("|");
                for (uint32_t a = count - 16; a < count; ++a) {
                    if (data[a] < 32 || data[a] > 126) {
                        printf(".");
                    } else {
                        printf("%c", data[a]);
                    }
                }
                printf("|\n");
                ++line;
                printf("|");
            }

            printf("%02X ", data[count]);

            if (count + 1 == length && length <= 16) {
                for (uint32_t b = count_pos + 1; b < 16; ++b) {
                    printf("   ");
                }
                printf("|");
                for (uint32_t a = 0; a < length; ++a) {
                    if (data[a] < 32 || data[a] > 126) {
                        printf(".");
                    }
                    else {
                        printf("%c", data[a]);
                    }
                }
                for (uint32_t c = count; c < 15; ++c) {
                    printf(" ");
                }
                printf("|\n");
            }

            if (count + 1 == length && length > 16) {
                for (uint32_t b = count_pos + 1; b < 16; ++b) {
                    printf("   ");
                }
                printf("|");
                uint32_t print = 0;
                for (uint32_t a = line * 16 - 16; a < length; ++a) {
                    if (data[a] < 32 || data[a] > 126) {
                        printf(".");
                    }
                    else {
                        printf("%c", data[a]);
                    }
                    ++print;
                }
                for (uint32_t c = print; c < 16; ++c) {
                    printf(" ");
                }
                printf("|\n");
            }

            ++count_pos;
        }
    }
    printf("-------------------------------------------------------------------\n\n");
}

void NetWrapper::CEventPipe::LibuvTimerCb(uv_timer_t * handle) {
    CContext * context = static_cast<CContext *>(handle->data);
    if (context) {
        std::shared_ptr<CEventPipe> event_pipe = context->event_pipe_.lock();
        if (event_pipe) {
            CAddress address;
            event_pipe->GetSocket()->GetRemoteAddress(address);
            OutputMsg(Logger::Info, "LibuvTimerCb, IP:%s", address.ToString().c_str());
            event_pipe->ShutdownImmediately();
            event_pipe->CallOnDisconnected(false);
            event_pipe->reg_timer_ = false;
        }
    }
}

void NetWrapper::CEventPipe::LibuvTimerCloseCb(uv_handle_t * handle) {
    delete handle;
}

void NetWrapper::CEventPipe::LibuvAllocCb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
    CContext * context = static_cast<CContext *>(handle->data);
    if (context) {
        std::shared_ptr<CEventPipe> event_pipe = context->event_pipe_.lock();
        if (event_pipe) {
            if (SocketOpt::S_CONNECTED == event_pipe->connect_state_ || SocketOpt::S_DISCONNECTING == event_pipe->connect_state_) {
                Packet * input = event_pipe->buffer_->input_;
                if (input->GetSize() - input->GetLength() < suggested_size) {
                    if (input->GetSize() < event_pipe->max_in_buffer_size_) {
                        input->Reserve(input->GetSize() + suggested_size);
                    }
                }
                buf->base = reinterpret_cast<char *>(input->GetPositionPtr(input->GetLength()));
                buf->len = input->GetSize() - input->GetLength();
            }
        }
    }
}

void NetWrapper::CEventPipe::LibuvReadCb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
    CContext * context = static_cast<CContext *>(stream->data);
    if (context) {
        std::shared_ptr<CEventPipe> event_pipe = context->event_pipe_.lock();
        if (event_pipe) {
            if (nread > 0) {
                if (SocketOpt::S_CONNECTED == event_pipe->connect_state_ || SocketOpt::S_DISCONNECTING == event_pipe->connect_state_) {
                    event_pipe->buffer_->input_->SetLength(event_pipe->buffer_->input_->GetLength() + nread);
                    event_pipe->OnNewDataReceived();
                }
            } else {
                event_pipe->HandleError(static_cast<int>(nread));
            }
        }
    }
}

void NetWrapper::CEventPipe::LibuvWriteCb(uv_write_t * req, int status) {
    WriteRequest * request = static_cast<WriteRequest *>(req->data);
    if (request) {
        std::shared_ptr<CEventPipe> event_pipe = request->event_pipe_.lock();
        if (event_pipe) {
            if (status < 0) {
                event_pipe->HandleError(status);
            } else {
                if (SocketOpt::S_CONNECTED == event_pipe->connect_state_ || SocketOpt::S_DISCONNECTING == event_pipe->connect_state_) {
                    event_pipe->OnSomeDataSent();
                    if (SocketOpt::S_DISCONNECTING == event_pipe->connect_state_) {
                        if (event_pipe->shutdown_) {
                            if (event_pipe->socket_.IsShutdownRead()) {
                                event_pipe->ShutdownImmediately();
                                event_pipe->CallOnDisconnected(false);
                            } else {
                                event_pipe->socket_.ShutdownWrite();
                            }
                        } else {
                            if (event_pipe->socket_.IsShutdownRead()) {
                                throw BaseException(__func__, "event_pipe->socket_.IsShutdownRead() == true");
                            }
                            event_pipe->ShutdownImmediately();
                            event_pipe->CallOnDisconnected(true);
                        }
                    } else if (event_pipe->buffer_->output_->GetLength() > 0) {
                        event_pipe->buffer_->InitWriteRequest();
                        int err = uv_write(event_pipe->buffer_->GetLibuvWriteReq(), reinterpret_cast<uv_stream_t *>(event_pipe->socket_.GetLibuvTcp()), event_pipe->buffer_->GetLibuvBuf(), 1, LibuvWriteCb);
                        if (0 != err) {
                            delete event_pipe->buffer_->request_;
                            event_pipe->buffer_->request_ = nullptr;
                            event_pipe->HandleError(err);
                        }
                    }
                }
            }
        }
        delete request;
    }
}