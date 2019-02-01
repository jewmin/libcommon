#ifndef __LIBCOMMON_NET_WRAPPER_REACTOR_HANDLER_H__
#define __LIBCOMMON_NET_WRAPPER_REACTOR_HANDLER_H__

#include "net_wrapper/event_reactor.h"

namespace NetWrapper {
    class CReactorHandler {
    public:
        CReactorHandler(CEventReactor * event_reactor);
        virtual ~CReactorHandler();
        virtual bool RegisterToReactor() = 0;
        virtual bool UnRegisterFromReactor() = 0;

        inline CEventReactor * GetEventReactor() const {
            return event_reactor_;
        }

        inline void SetEventReactor(CEventReactor * event_reactor) {
            if (!event_reactor) {
                throw BaseException(__func__, "event_reactor == nullptr");
            }
            event_reactor_ = event_reactor;
        }

    private:
        CEventReactor * event_reactor_;
    };
}

#endif