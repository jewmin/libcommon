#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_HANDLER_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_HANDLER_H__

#include "net_wrapper/connection.h"

namespace NetWrapper {
    class CEventHandler {
    public:
        virtual int OnConnected(CConnection * connection) = 0;
        virtual int OnConnectFailed(CConnection * connection, EPipeConnFailedReason reason) = 0;
        virtual int OnDisconnect(CConnection * connection, EPipeDisconnectReason reason, bool remote) = 0;
        virtual int OnDisconnected(CConnection * connection, EPipeDisconnectReason reason, bool remote) = 0;
        virtual int OnNewDataReceived(CConnection * connection) = 0;
        virtual int OnSomeDataSent(CConnection * connection) = 0;
        virtual int BeforeWrite(const uint8_t * data, size_t len) = 0;
        virtual int AfterWrite(CConnection * connection, size_t len) = 0;
        virtual void OnTick(CConnection * connection) = 0;

        inline std::string GetName() const {
            return name_;
        }

    private:
        std::string name_;
    };
}

#endif