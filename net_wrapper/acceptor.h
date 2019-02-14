#ifndef __LIBCOMMON_NET_WRAPPER_ACCEPTOR_H__
#define __LIBCOMMON_NET_WRAPPER_ACCEPTOR_H__

#include "net_wrapper/socket.h"
#include "net_wrapper/address.h"
#include "net_wrapper/event_pipe.h"
#include "net_wrapper/reactor_handler.h"

namespace NetWrapper {
    class CAcceptor : public CReactorHandler {
    public:
        class CContext {
        public:
            CContext(const std::shared_ptr<CAcceptor> & acceptor);
            ~CContext();
            std::weak_ptr<CAcceptor> acceptor_;
        };

        CAcceptor(CEventReactor * event_reactor);
        virtual ~CAcceptor();
        bool RegisterToReactor() override;
        bool UnRegisterFromReactor() override;
        virtual void SetMaxOutBufferSize(uint32_t max_out_buffer_size) = 0;
        virtual void SetMaxInBufferSize(uint32_t max_in_buffer_size) = 0;
        virtual void MakeEventPipe(CEventPipe * & event_pipe) = 0;
        virtual void OnAccepted(CEventPipe * event_pipe) = 0;
        bool Open(const CAddress & address);
        bool Open(const char * ip, uint16_t port);
        bool Open6(const CAddress & address);
        bool Open6(const char * ip, uint16_t port);
        void Close(void);
        void AcceptEventPipe(CEventPipe * event_pipe, uv_tcp_t * tcp);
        bool ActivateEventPipe(CEventPipe * event_pipe);
        void Accept();

    protected:
        bool OpenImpl(const CAddress & address, EAddressFamily family);
        static void Accept(uv_stream_t * stream, int status);

    private:
        CAddress * address_;
        CSocket socket_;
        bool open_;
    };
}

#endif