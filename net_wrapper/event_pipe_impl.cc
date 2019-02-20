#include "net_wrapper/event_pipe_impl.h"

NetWrapper::CEventPipeImpl::CEventPipeImpl(IEventPipeHandler * handler, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CEventPipe(max_out_buffer_size, max_in_buffer_size), handler_(handler) {
    if (!handler_) {
        throw BaseException(__func__, "handler_ == nullptr");
    }
}

NetWrapper::CEventPipeImpl::~CEventPipeImpl() {
    ShutdownImmediately();
}

void NetWrapper::CEventPipeImpl::OnConnected() {
    handler_->OnConnected();
}

void NetWrapper::CEventPipeImpl::OnConnectFailed(EPipeConnFailedReason reason) {
    handler_->OnConnectFailed(reason);
}

void NetWrapper::CEventPipeImpl::OnDisconnect(EPipeDisconnectReason reason, bool remote) {
    handler_->OnDisconnect(reason, remote);
}

void NetWrapper::CEventPipeImpl::OnDisconnected(EPipeDisconnectReason reason, bool remote) {
    handler_->OnDisconnected(reason, remote);
}

void NetWrapper::CEventPipeImpl::OnNewDataReceived() {
    handler_->OnNewDataReceived();
}

void NetWrapper::CEventPipeImpl::OnSomeDataSent() {
    handler_->OnSomeDataSent();
}

void NetWrapper::CEventPipeImpl::OnError(int error) {
    handler_->OnError(error);
}