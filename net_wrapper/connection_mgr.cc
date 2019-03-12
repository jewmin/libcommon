#include "net_wrapper/connection_mgr.h"

uint32_t NetWrapper::CConnectionMgr::s_num_created_;

NetWrapper::CConnectionMgr::CConnectionMgr(std::string name)
    : handlers_(new std::vector<CEventHandler *>()), name_(name)
    , connections_(new std::unordered_map<uint32_t, CConnection *>())
    , need_delete_list_(new std::list<uint32_t>()), need_shutdown_list_(new std::list<uint32_t>()) {

}

NetWrapper::CConnectionMgr::~CConnectionMgr() {
    if (connections_) {
        for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
            if (it->second) {
                delete it->second;
                it->second = nullptr;
            }
        }
        delete connections_;
        connections_ = nullptr;
    }

    if (need_shutdown_list_) {
        delete need_shutdown_list_;
        need_shutdown_list_ = nullptr;
    }

    if (need_delete_list_) {
        delete need_delete_list_;
        need_delete_list_ = nullptr;
    }

    if (handlers_) {
        delete handlers_;
        handlers_ = nullptr;
    }
}

void NetWrapper::CConnectionMgr::Update() {
    CleanDeathPipe();
}

void NetWrapper::CConnectionMgr::AddEventHandler(const std::string & handler) {
    std::map<std::string, CEventHandler *>::iterator it = CEventHandlerRegister::map_handlers_.find(handler);
    if (it == CEventHandlerRegister::map_handlers_.end()) {
        throw BaseException(__func__, "it == CEventHandlerRegister::map_handlers_.end()");
    }
    handlers_->push_back(it->second);
}

void NetWrapper::CConnectionMgr::SetEventHandlers(const std::vector<std::string> & handlers) {
    handlers_->clear();
    for (size_t i = 0; i < handlers.size(); ++i) {
        AddEventHandler(handlers[i]);
    }
}

void NetWrapper::CConnectionMgr::AddEventHandler(CEventHandler * handler) {
    if (!handler) {
        throw BaseException(__func__, "handler == nullptr");
    }
    handlers_->push_back(handler);
}

void NetWrapper::CConnectionMgr::SetEventHandlers(std::vector<CEventHandler *> handlers) {
    handlers_->clear();
    for (size_t i = 0; i < handlers.size(); ++i) {
        AddEventHandler(handlers[i]);
    }
}

void NetWrapper::CConnectionMgr::DumpEventHandler() {
    OutputMsg(Logger::Info, "-----------------------------");
    for (size_t i = 0; i < handlers_->size(); ++i) {
        OutputMsg(Logger::Info, (*handlers_)[i]->GetName().c_str());
    }
}

uint32_t NetWrapper::CConnectionMgr::RegisterConn(CConnection * connection) {
    if (connection->GetIsRegister2Mgr()) {
        return connection->GetConnectionID();
    }

    if (!connection->GetMgr()) {
        throw BaseException(__func__, "connection->GetMgr() == nullptr");
    }

    uint32_t connection_id = connection->GetConnectionID();
    if (0 == connection_id) {
        connection_id = ++s_num_created_;
        connection->SetConnectionID(connection_id);
    }
    connections_->insert(std::pair<uint32_t, CConnection *>(connection_id, connection));
    connection->SetIsRegister2Mgr(true);
    return connection_id;
}

void NetWrapper::CConnectionMgr::UnRegisterConn(CConnection * connection) {
    if (connection) {
        uint32_t connection_id = connection->GetConnectionID();
        need_delete_list_->push_back(connection_id);
    }
}

void NetWrapper::CConnectionMgr::NeedToShutdownByID(uint32_t connection_id) {
    CConnection * connection = GetConnectionByID(connection_id);
    if (connection) {
        ShutDownConnByID(connection_id);
    }
}

void NetWrapper::CConnectionMgr::NeedToShutdownByID(uint32_t connection_id, const char * reason) {
    CConnection * connection = GetConnectionByID(connection_id);
    if (connection) {
        OutputMsg(Logger::Info, "%s.%d:--关闭连接[ID:%u] 原因:%s--", __FILE__, __LINE__, connection_id, reason);
        ShutDownConnByID(connection_id);
    }
}

bool NetWrapper::CConnectionMgr::ConnIsValid(uint32_t connection_id) {
    std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->find(connection_id);
    if (it != connections_->end()) {
        return it->second && it->second->GetConnectState() == SocketOpt::S_CONNECTED;
    }
    return false;
}

void NetWrapper::CConnectionMgr::SendToAllClient(const uint8_t * data, size_t len) {
    for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
        if (it->second) {
            it->second->Send(data, len);
        }
    }
}

void NetWrapper::CConnectionMgr::ShutDownAllConnection() {
    OutputMsg(Logger::Info, "--- %s:ShutDownAllConnection ---", name_.c_str());
    for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
        if (it->second) {
            it->second->Shutdown();
        }
    }
}

void NetWrapper::CConnectionMgr::ShutDownConnByID(uint32_t connection_id) {
    CConnection * connection = GetConnectionByID(connection_id);
    if (connection) {
        connection->Shutdown();
    }
}

void NetWrapper::CConnectionMgr::InsertToNeedToShutdownList(uint32_t connection_id) {
    need_shutdown_list_->push_back(connection_id);
}

void NetWrapper::CConnectionMgr::CleanDeathPipe() {
    while (!need_shutdown_list_->empty()) {
        uint32_t connection_id = need_shutdown_list_->front();
        NeedToShutdownByID(connection_id);
        need_shutdown_list_->pop_front();
        OutputMsg(Logger::Error, "%s.%d: %s:缓冲区满了，NeedToShutdownByID，Conn ID =%u", __FILE__, __LINE__, name_.c_str(), connection_id);
    }

    while (!need_delete_list_->empty()) {
        uint32_t connection_id = need_delete_list_->front();
        std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->find(connection_id);
        if (it != connections_->end()) {
            if (it->second) {
                delete it->second;
                connections_->erase(it);
            }
        }
        need_delete_list_->pop_front();
        OutputMsg(Logger::Info, "%s.%d: %s:删除连接，Conn ID =%u", __FILE__, __LINE__, name_.c_str(), connection_id);
    }
}

size_t NetWrapper::CConnectionMgr::GetConnectionCount() {
    return connections_->size();
}

NetWrapper::CConnection * NetWrapper::CConnectionMgr::GetConnectionByID(uint32_t connection_id) {
    std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->find(connection_id);
    if (it != connections_->end()) {
        if (it->second) {
            return it->second;
        }
    }
    return nullptr;
}

NetWrapper::CEventHandler * NetWrapper::CConnectionMgr::GetCEventHandler(int index) {
    return (*handlers_)[index];
}

size_t NetWrapper::CConnectionMgr::GetAllConnInBufferSize() {
    size_t size = 0;
    for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
        if (it->second) {
            size += it->second->GetInBufferSize();
        }
    }
    return size;
}

size_t NetWrapper::CConnectionMgr::GetAllConnOutBufferSize() {
    size_t size = 0;
    for (std::unordered_map<uint32_t, CConnection *>::iterator it = connections_->begin(); it != connections_->end(); ++it) {
        if (it->second) {
            size += it->second->GetOutBufferSize();
        }
    }
    return size;
}

size_t NetWrapper::CConnectionMgr::GetOneConnOutBufferUsedSize(uint32_t connection_id) {
    CConnection * connection = GetConnectionByID(connection_id);
    if (connection) {
        return connection->GetOutBufferUsedSize();
    } else {
        return 0;
    }
}