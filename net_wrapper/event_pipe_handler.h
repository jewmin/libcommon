#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_HANDLER_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_PIPE_HANDLER_H__

#include "net_wrapper/event_pipe.h"

namespace NetWrapper {
    class IEventPipeHandler {
    public:
        virtual void OnConnected() = 0;
        virtual void OnConnectFailed(EPipeConnFailedReason reason) = 0;
        virtual void OnDisconnect(EPipeDisconnectReason reason, bool remote) = 0;
        virtual void OnDisconnected(EPipeDisconnectReason reason, bool remote) = 0;
        virtual void OnNewDataReceived() = 0;
        virtual void OnSomeDataSent() = 0;
        virtual void OnError(int error) = 0;
    };
}

#endif