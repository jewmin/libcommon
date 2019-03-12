#include "net_wrapper/server.h"
#include "net_wrapper/acceptor_impl.h"

NetWrapper::CServer::CServer(std::string name, CEventReactor * event_reactor, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CConnectionMgr(name), event_reactor_(event_reactor), max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {

}

NetWrapper::CServer::~CServer() {

}

template<class TConnection>
bool NetWrapper::CServer::BeginService(std::string ip, uint16_t port) {
    return BeginServiceImpl<TConnection>(ip, port, ADDRESSFAMILY_IPV4);
}

template<class TConnection>
bool NetWrapper::CServer::BeginService6(std::string ip, uint16_t port) {
    return BeginServiceImpl<TConnection>(ip, port, ADDRESSFAMILY_IPV6);
}

template<class TConnection>
bool NetWrapper::CServer::BeginServiceImpl(std::string ip, uint16_t port, EAddressFamily family) {
    if (listen_) {
        return false;
    }

    if (0 == port) {
        throw BaseException(__func__, "port == 0");
    }

    if ("" == ip) {
        throw BaseException(__func__, "ip == \"\"");
    }

    if (acceptor_) {
        throw BaseException(__func__, "acceptor_ != nullptr");
    }

    address_.SetAddress(ip.c_str());
    address_.setPort(port);
    acceptor_ = new TAcceptorImpl<TConnection>(event_reactor_, this, max_out_buffer_size_, max_in_buffer_size_);
    if (ADDRESSFAMILY_IPV4 == family) {
        listen_ = acceptor_->Open(address_);
    } else {
        listen_ = acceptor_->Open6(address_);
    }

    if (listen_) {
        return true;
    } else {
        delete acceptor_;
        acceptor_ = nullptr;
        return false;
    }
}

bool NetWrapper::CServer::EndService() {
    if (!listen_) {
        return false;
    }

    listen_ = false;
    if (!acceptor_) {
        throw BaseException(__func__, "acceptor_ == nullptr");
    }

    acceptor_->Close();
    delete acceptor_;
    acceptor_ = nullptr;
    return true;
}

void NetWrapper::CServer::SetRecvBufferSize(uint32_t max_in_buffer_size) {
    max_in_buffer_size_ = max_in_buffer_size;
    if (acceptor_) {
        acceptor_->SetMaxInBufferSize(max_in_buffer_size);
        for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
            if (it->second) {
                it->second->SetMaxInBufferSize(max_in_buffer_size);
            }
        }
    }
}

void NetWrapper::CServer::SetSendBufferSize(uint32_t max_out_buffer_size) {
    max_out_buffer_size_ = max_out_buffer_size;
    if (acceptor_) {
        acceptor_->SetMaxOutBufferSize(max_out_buffer_size);
        for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
            if (it->second) {
                it->second->SetMaxOutBufferSize(max_out_buffer_size);
            }
        }
    }
}