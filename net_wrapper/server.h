#ifndef __LIBCOMMON_NET_WRAPPER_SERVER_H__
#define __LIBCOMMON_NET_WRAPPER_SERVER_H__

#include "net_wrapper/acceptor.h"
#include "net_wrapper/connection_mgr.h"

namespace NetWrapper {
    class CServer : public CConnectionMgr {
    public:
        CServer(std::string name, CEventReactor * event_reactor, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size);
        virtual ~CServer();

        template<class TConnection>
        bool BeginService(std::string ip, uint16_t port);
        template<class TConnection>
        bool BeginService6(std::string ip, uint16_t port);
        bool EndService();
        void SetRecvBufferSize(uint32_t max_in_buffer_size);
        void SetSendBufferSize(uint32_t max_out_buffer_size);

    protected:
        template<class TConnection>
        bool BeginServiceImpl(std::string ip, uint16_t port, EAddressFamily family);

    private:
        bool listen_;
        CAddress address_;
        CAcceptor * acceptor_;
        CEventReactor * event_reactor_;
        uint32_t max_out_buffer_size_;
        uint32_t max_in_buffer_size_;
    };

}

#endif