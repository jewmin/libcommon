#ifndef __LIBCOMMON_NET_WRAPPER_CONNECTION_MGR_H__
#define __LIBCOMMON_NET_WRAPPER_CONNECTION_MGR_H__

#include <unordered_map>
#include "net_wrapper/connection.h"
#include "net_wrapper/event_handler.h"

namespace NetWrapper {
    class CConnectionMgr {
    public:
        CConnectionMgr(std::string name);
        virtual ~CConnectionMgr();
        virtual void Update();
        virtual void AddEventHandler(const std::string & handler);
        virtual void SetEventHandlers(const std::vector<std::string> & handlers);
        virtual void AddEventHandler(CEventHandler * handler);
        virtual void SetEventHandlers(std::vector<CEventHandler *> handlers);
        virtual void DumpEventHandler();
        virtual uint32_t RegisterConn(CConnection * connection);
        virtual void UnRegisterConn(CConnection * connection);
        virtual void NeedToShutdownByID(uint32_t connection_id);
        virtual void NeedToShutdownByID(uint32_t connection_id, const char * reason);
        virtual bool ConnIsValid(uint32_t connection_id);
        virtual void SendToAllClient(const uint8_t * data, size_t len);
        virtual void ShutDownAllConnection();
        virtual void ShutDownConnByID(uint32_t connection_id);
        virtual void InsertToNeedToShutdownList(uint32_t connection_id);
        virtual void CleanDeathPipe();

        int GetConnectionCount();
        CConnection * GetConnectionByID(uint32_t connection_id);
        CEventHandler * GetCEventHandler(int index);
        size_t GetAllConnInBufferSize();
        size_t GetAllConnOutBufferSize();
        size_t GetOneConnOutBufferUsedSize(uint32_t connection_id);

    private:
        std::vector<CEventHandler *> * handlers_;
        int handler_count_;
        std::string name_;
        std::unordered_map<uint32_t, CConnection *> * connections_;
        int connection_count_;
        std::list<uint32_t> * need_delete_list_;
        std::list<uint32_t> * need_shutdown_list_;
    };
}

#endif