#ifndef __LIBCOMMON_NET_WRAPPER_TACCEPTOR_IMPL_H__
#define __LIBCOMMON_NET_WRAPPER_TACCEPTOR_IMPL_H__

#include "net_wrapper/acceptor.h"

namespace NetWrapper {
    class CServer;
    template<class TConnection>
    class TAcceptorImpl : public CAcceptor {
    public:
        TAcceptorImpl(CEventReactor * event_reactor, CServer * server, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size);
        virtual ~TAcceptorImpl();
        void SetMaxOutBufferSize(uint32_t max_out_buffer_size) override;
        void SetMaxInBufferSize(uint32_t max_in_buffer_size) override;
        void MakeEventPipe(CEventPipe * & event_pipe) override;
        void OnAccepted(CEventPipe * event_pipe) override;

    private:
        CServer * server_;
        uint32_t max_out_buffer_size_;
        uint32_t max_in_buffer_size_;
    };
}

#endif