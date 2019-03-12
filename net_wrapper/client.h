#ifndef __LIBCOMMON_NET_WRAPPER_CLIENT_H__
#define __LIBCOMMON_NET_WRAPPER_CLIENT_H__

#include "net_wrapper/connector.h"
#include "net_wrapper/connection.h"
#include "net_wrapper/connection_mgr.h"

namespace NetWrapper {
    class CClient : public CConnectionMgr {
    public:
        CClient(std::string name, CConnector * connector, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size);
        virtual ~CClient();
        virtual uint32_t Connect(const CAddress & address);
        virtual bool Connect(const CAddress & address, uint32_t & connection_id);
        virtual bool Connect(const CAddress & address, uint32_t & connection_id, CConnection * connection);
        virtual uint32_t Connect6(const CAddress & address);
        virtual bool Connect6(const CAddress & address, uint32_t & connection_id);
        virtual bool Connect6(const CAddress & address, uint32_t & connection_id, CConnection * connection);
        void ClearConn2Server();

    protected:
        virtual bool ConnectImpl(const CAddress & address, uint32_t & connection_id, CConnection * connection, EAddressFamily family);

    private:
        CConnector * connector_;
        uint32_t max_out_buffer_size_;
        uint32_t max_in_buffer_size_;
    };
}

#endif