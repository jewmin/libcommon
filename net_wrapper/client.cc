#include "net_wrapper/client.h"

NetWrapper::CClient::CClient(std::string name, CConnector * connector, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size)
    : CConnectionMgr(name), connector_(connector), max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {
    if (!connector_) {
        throw BaseException(__func__, "connector_ == nullptr");
    }
}

NetWrapper::CClient::~CClient() {

}

uint32_t NetWrapper::CClient::Connect(const CAddress & address) {
    uint32_t connection_id;
    CConnection * connection = new CConnection(this, max_out_buffer_size_, max_in_buffer_size_);
    ConnectImpl(address, connection_id, connection, ADDRESSFAMILY_IPV4);
    return connection_id;
}

bool NetWrapper::CClient::Connect(const CAddress & address, uint32_t & connection_id) {
    CConnection * connection = new CConnection(this, max_out_buffer_size_, max_in_buffer_size_);
    return ConnectImpl(address, connection_id, connection, ADDRESSFAMILY_IPV4);
}

bool NetWrapper::CClient::Connect(const CAddress & address, uint32_t & connection_id, CConnection * connection) {
    return ConnectImpl(address, connection_id, connection, ADDRESSFAMILY_IPV4);
}

uint32_t NetWrapper::CClient::Connect6(const CAddress & address) {
    uint32_t connection_id;
    CConnection * connection = new CConnection(this, max_out_buffer_size_, max_in_buffer_size_);
    ConnectImpl(address, connection_id, connection, ADDRESSFAMILY_IPV6);
    return connection_id;
}

bool NetWrapper::CClient::Connect6(const CAddress & address, uint32_t & connection_id) {
    CConnection * connection = new CConnection(this, max_out_buffer_size_, max_in_buffer_size_);
    return ConnectImpl(address, connection_id, connection, ADDRESSFAMILY_IPV6);
}

bool NetWrapper::CClient::Connect6(const CAddress & address, uint32_t & connection_id, CConnection * connection) {
    return ConnectImpl(address, connection_id, connection, ADDRESSFAMILY_IPV6);
}

bool NetWrapper::CClient::ConnectImpl(const CAddress & address, uint32_t & connection_id, CConnection * connection, EAddressFamily family) {
    if (!connection) {
        throw BaseException(__func__, "connection == nullptr");
    }

    int ret;
    connection->SetMgr(this);
    connection_id = RegisterConn(connection);
    if (ADDRESSFAMILY_IPV4 == family) {
        connector_->Connect(connection->GetRealEventPipe(), address, ret);
    } else {
        connector_->Connect6(connection->GetRealEventPipe(), address, ret);
    }
    if (2 != ret) {
        return true;
    }

    std::stringstream ss;
    OutputMsg(Logger::Warn, "%s.%d: --%s:建立连接失败!--", __FILE__, __LINE__, GetName().c_str());
    return false;
}

void NetWrapper::CClient::ClearConn2Server() {
    ShutDownAllConnection();
}