#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_IMPL_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_IMPL_H__

#include "net_wrapper/event_pipe.h"
#include "net_wrapper/event_pipe_handler.h"

namespace NetWrapper {
    class CEventPipeImpl : public CEventPipe {
    public:
        CEventPipeImpl(IEventPipeHandler * handler, uint32_t max_out_buffer_size, uint32_t max_in_buffer_size);
        virtual ~CEventPipeImpl();
        virtual void OnConnected();
        virtual void OnConnectFailed(EPipeConnFailedReason reason);
        virtual void OnDisconnect(EPipeDisconnectReason reason, bool remote);
        virtual void OnDisconnected(EPipeDisconnectReason reason, bool remote);
        virtual void OnNewDataReceived();
        virtual void OnSomeDataSent();
        virtual void OnError(int error);

    private:
        IEventPipeHandler * handler_;
    };
}

#endif