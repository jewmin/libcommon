#include "net_wrapper/connection.h"
#include "net_wrapper/connection_mgr.h"

NetWrapper::CConnection::CConnection(CConnectionMgr * mgr, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : connection_mgr_(mgr), connection_id_(0), register_mgr_(false), raw_send_recv_(false), need_to_shutdown_(false) {
    if (!connection_mgr_) {
        throw BaseException(__func__, "connection_mgr_ == nullptr");
    }

    event_pipe_ = new CEventPipeImpl(this, max_out_buffer_size, max_in_buffer_size);
    if (!event_pipe_) {
        throw BaseException(__func__, "event_pipe_ == nullptr");
    }
}

NetWrapper::CConnection::~CConnection() {
    if (event_pipe_) {
        delete event_pipe_;
        event_pipe_ = nullptr;
    }
}

void NetWrapper::CConnection::OnConnected() {
    if (!register_mgr_) {
        connection_mgr_->RegisterConn(this);
    }

    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->OnConnected(this);
        if (1 == ret) {
            break;
        } else if (2 == ret) {
            NeedToShutdown();
            return;
        } else if (0 != ret) {
            throw BaseException(__func__, "handler->OnConnected(this) != 0");
        }
    }
}

void NetWrapper::CConnection::OnConnectFailed(EPipeConnFailedReason reason) {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->OnConnectFailed(this, reason);
        if (0 != ret) {
            break;
        }
    }
    connection_mgr_->UnRegisterConn(this);
}

void NetWrapper::CConnection::OnDisconnect(EPipeDisconnectReason reason, bool remote) {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->OnDisconnect(this, reason, remote);
        if (0 != ret) {
            break;
        }
    }
}

void NetWrapper::CConnection::OnDisconnected(EPipeDisconnectReason reason, bool remote) {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->OnDisconnected(this, reason, remote);
        if (0 != ret) {
            break;
        }
    }
    connection_mgr_->UnRegisterConn(this);
}

void NetWrapper::CConnection::OnNewDataReceived() {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->OnNewDataReceived(this);
        if (1 == ret) {
            break;
        } else if (2 == ret) {
            NeedToShutdown();
            return;
        } else if (0 != ret) {
            throw BaseException(__func__, "handler->OnNewDataReceived(this) != 0");
        }
    }
}

void NetWrapper::CConnection::OnSomeDataSent() {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->OnSomeDataSent(this);
        if (1 == ret) {
            break;
        } else if (2 == ret) {
            NeedToShutdown();
            return;
        } else if (0 != ret) {
            throw BaseException(__func__, "handler->OnSomeDataSent(this) != 0");
        }
    }
}

void NetWrapper::CConnection::OnError(int error) {
    if (UV_EOF == error) {
        OutputMsg(Logger::Info, "[net] 连接[%u] OnError(%d):收到对端 EOF，正常断开", GetConnectionID(), error);
    } else {
        OutputMsg(Logger::Warn, "[net] 连接[%u] OnError(%d):连接错误:[%s]", GetConnectionID(), error, uv_strerror(error));
    }
}

void NetWrapper::CConnection::BeforeWrite(const uint8_t * data, size_t len) {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->BeforeWrite(data, len);
        if (1 == ret) {
            break;
        } else if (2 == ret) {
            OutputMsg(Logger::Error, "<Connection:%u>EventHandler(%s)->BeforeWrite 返回 HANDLER_ERROR", GetConnectionID(), handler->GetName().c_str());
            NeedToShutdown("BeforeWrite 返回 HANDLER_ERROR");
            return;
        } else if (0 != ret) {
            throw BaseException(__func__, "handler->BeforeWrite(data, len) != 0");
        }
    }
}

void NetWrapper::CConnection::AfterWrite(size_t len) {
    int ret;
    for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
        CEventHandler * handler = (*connection_mgr_->handlers_)[i];
        ret = handler->AfterWrite(this, len);
        if (1 == ret) {
            break;
        } else if (2 == ret) {
            OutputMsg(Logger::Error, "<Connection:%u>EventHandler(%s)->AfterWrite 返回 HANDLER_ERROR", GetConnectionID(), handler->GetName().c_str());
            NeedToShutdown("AfterWrite 返回 HANDLER_ERROR");
            return;
        } else if (0 != ret) {
            throw BaseException(__func__, "handler->AfterWrite(this, len) != 0");
        }
    }
}

size_t NetWrapper::CConnection::SafeWrite(const uint8_t * data, size_t len) {
    if (need_to_shutdown_) {
        return 0;
    }

    int ret = event_pipe_->Write(data, len);
    if (-2 == ret) {
        OutputMsg(Logger::Error, "<Connection:%u>写缓冲区超出上限，没用写入任何数据", GetConnectionID());
        need_to_shutdown_ = true;
        connection_mgr_->InsertToNeedToShutdownList(connection_id_);
    } else if (UV_ENOMEM == ret) {
        OutputMsg(Logger::Error, "<Connection:%u>写入失败，内存不足", GetConnectionID());
        connection_mgr_->NeedToShutdownByID(connection_id_);
    } else if (0 == ret) {
        return len;
    }
    return 0;
}

int NetWrapper::CConnection::Write(const uint8_t * data, size_t len) {
    return event_pipe_->Write(data, len);
}

int NetWrapper::CConnection::Send(const uint8_t * data, size_t len) {
    if (!need_to_shutdown_) {
        int ret;
        for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
            CEventHandler * handler = (*connection_mgr_->handlers_)[i];
            ret = handler->BeforeWrite(data, len);
            if (1 == ret) {
                break;
            } else if (2 == ret) {
                OutputMsg(Logger::Error, "<Connection:%u>EventHandler(%s)->BeforeWrite 返回 HANDLER_ERROR", GetConnectionID(), handler->GetName().c_str());
                NeedToShutdown("BeforeWrite 返回 HANDLER_ERROR");
                return ret;
            } else if (0 != ret) {
                throw BaseException(__func__, "handler->BeforeWrite(data, len) != 0");
            }
        }
        ret = event_pipe_->Write(data, len);
        if (-2 == ret) {
            OutputMsg(Logger::Error, "<Connection:%u>写缓冲区超出上限，没用写入任何数据", GetConnectionID());
            need_to_shutdown_ = true;
            connection_mgr_->InsertToNeedToShutdownList(connection_id_);
        } else if (UV_ENOMEM == ret) {
            OutputMsg(Logger::Error, "<Connection:%u>写入失败，内存不足", GetConnectionID());
            connection_mgr_->NeedToShutdownByID(connection_id_);
        } else if (0 == ret) {
            for (size_t i = 0; i < connection_mgr_->handlers_->size(); ++i) {
                CEventHandler * handler = (*connection_mgr_->handlers_)[i];
                ret = handler->AfterWrite(this, len);
                if (1 == ret) {
                    return ret;
                } else if (2 == ret) {
                    OutputMsg(Logger::Error, "<Connection:%u>EventHandler(%s)->AfterWrite 返回 HANDLER_ERROR", GetConnectionID(), handler->GetName().c_str());
                    NeedToShutdown("AfterWrite 返回 HANDLER_ERROR");
                    return ret;
                } else if (0 != ret) {
                    throw BaseException(__func__, "handler->AfterWrite(this, len) != 0");
                }
            }
        }
    }
    return 1;
}

void NetWrapper::CConnection::NeedToShutdown() {
    connection_mgr_->NeedToShutdownByID(connection_id_);
}

void NetWrapper::CConnection::NeedToShutdown(const char * reason) {
    connection_mgr_->NeedToShutdownByID(connection_id_, reason);
}

void NetWrapper::CConnection::Shutdown() {
    event_pipe_->Shutdown(false);
}

void NetWrapper::CConnection::ShutdownNow() {
    event_pipe_->Shutdown(true);
}

void NetWrapper::CConnection::OnPreAppShutDown() {

}

std::string NetWrapper::CConnection::GetInfo() {
    std::stringstream ss;
    CAddress address;
    CSocket * socket = GetSocket();
    socket->GetRemoteAddress(address);
    ss << address.GetAddress() << ":" << address.getPort();
    return ss.str();
}

uint8_t * NetWrapper::CConnection::ReverseGetOutputData(size_t len) {
    return event_pipe_->ReverseGetOutputData(len);
}

uint8_t * NetWrapper::CConnection::GetRecvData() const {
    return event_pipe_->GetRecvData();
}

size_t NetWrapper::CConnection::GetRecvDataSize() const {
    return event_pipe_->GetRecvDataSize();
}

void NetWrapper::CConnection::PopRecvData(size_t len) {
    event_pipe_->PopRecvData(len);
}

void NetWrapper::CConnection::SetMaxOutBufferSize(uint32_t size) {
    event_pipe_->SetMaxOutBufferSize(size);
}

void NetWrapper::CConnection::SetMaxInBufferSize(uint32_t size) {
    event_pipe_->SetMaxInBufferSize(size);
}

size_t NetWrapper::CConnection::GetInBufferSize() {
    return event_pipe_->GetInBufferSize();
}

size_t NetWrapper::CConnection::GetOutBufferSize() {
    return event_pipe_->GetOutBufferSize();
}

size_t NetWrapper::CConnection::GetOutBufferUsedSize() {
    return event_pipe_->GetOutBufferUsedSize();
}

void NetWrapper::CConnection::DumpOutBuf() {
    event_pipe_->DumpOutBuf();
}

void NetWrapper::CConnection::DumpInBuf() {
    event_pipe_->DumpInBuf();
}