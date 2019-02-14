#ifndef __LIBCOMMON_NET_WRAPPER_CONNECTOR_H__
#define __LIBCOMMON_NET_WRAPPER_CONNECTOR_H__

#include "net_wrapper/event_pipe.h"
#include "net_wrapper/reactor_handler.h"

namespace NetWrapper {
    class CConnector : public CReactorHandler {
    public:
        class CContext {
        public:
            CContext(const std::shared_ptr<CConnector> & connector, const std::shared_ptr<CEventPipe> & event_pipe);
            ~CContext();
            std::weak_ptr<CConnector> connector_;
            std::weak_ptr<CEventPipe> event_pipe_;
        };

        CConnector(CEventReactor * event_reactor);
        virtual ~CConnector();
        bool RegisterToReactor() override;
        bool UnRegisterFromReactor() override;
        void Connect(CEventPipe * event_pipe, const CAddress & address, int & ret);
        void Connect(CEventPipe * event_pipe, const char * ip, uint16_t port, int & ret);
        void Connect6(CEventPipe * event_pipe, const CAddress & address, int & ret);
        void Connect6(CEventPipe * event_pipe, const char * ip, uint16_t port, int & ret);
        bool ActivateEventPipe(CEventPipe * event_pipe);
        void OnOneConnectSuccess(CEventPipe * event_pipe);

    protected:
        void ConnectImpl(CEventPipe * event_pipe, const CAddress & address, int & ret, EAddressFamily family);
        static void LibuvCb(uv_connect_t * req, int status);
    };
}

#endif