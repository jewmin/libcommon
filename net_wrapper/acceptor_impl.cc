#include "net_wrapper/acceptor_impl.h"

template<class TConnection>
NetWrapper::TAcceptorImpl<TConnection>::TAcceptorImpl(CEventReactor * event_reactor, CServer * server, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CAcceptor(event_reactor), server_(server), max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {

}

template<class TConnection>
NetWrapper::TAcceptorImpl<TConnection>::~TAcceptorImpl() {

}

template<class TConnection>
void NetWrapper::TAcceptorImpl<TConnection>::SetMaxOutBufferSize(uint32_t max_out_buffer_size) {
    max_out_buffer_size_ = max_out_buffer_size;
}

template<class TConnection>
void NetWrapper::TAcceptorImpl<TConnection>::SetMaxInBufferSize(uint32_t max_in_buffer_size) {
    max_in_buffer_size_ = max_in_buffer_size;
}

template<class TConnection>
void NetWrapper::TAcceptorImpl<TConnection>::MakeEventPipe(CEventPipe * & event_pipe) {
    TConnection * connection = new TConnection(server_, max_out_buffer_size_, max_in_buffer_size_);
    event_pipe = connection->GetEventPipe();
}

template<class TConnection>
void NetWrapper::TAcceptorImpl<TConnection>::OnAccepted(CEventPipe * event_pipe) {

}