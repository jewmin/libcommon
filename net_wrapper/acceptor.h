#ifndef __LIBCOMMON_NET_WRAPPER_ACCEPTOR_H__
#define __LIBCOMMON_NET_WRAPPER_ACCEPTOR_H__

#include "net_wrapper/socket.h"
#include "net_wrapper/address.h"
#include "net_wrapper/event_pipe.h"
#include "net_wrapper/reactor_handler.h"

namespace NetWrapper {
    class CAcceptor : public CReactorHandler {
    public:
        CAcceptor(CEventReactor * event_reactor);
        virtual ~CAcceptor();
        bool RegisterToReactor() override;
        bool UnRegisterFromReactor() override;
        virtual void SetMaxOutBufferSize(uint32_t max_out_buffer_size) = 0;
        virtual void SetMaxInBufferSize(uint32_t max_in_buffer_size) = 0;
        virtual void MakeEventPipe(CEventPipe * & event_pipe) = 0;
        virtual void OnAccepted(CEventPipe * event_pipe) = 0;
        bool Open(const char * ip, uint16_t port);
        bool Open(const CAddress & address);
        void Close(void);
        void AcceptEventPipe(CEventPipe * event_pipe);
        bool ActivateEventPipe(CEventPipe * event_pipe);

    protected:
        static void Accept(uv_stream_t * stream, int status);

    private:
        CAddress * address_;
        CSocket socket_;
        bool open_;
    };
}

#endif