#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_HANDLER_REGISTER_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_HANDLER_REGISTER_H__

#include <map>
#include "net_wrapper/event_handler.h"

namespace NetWrapper {
    class CEventHandlerRegister {
    public:
        static void RegisterHandler(CEventHandler * handler);
        static void UnRegisterAll();
        static std::map<std::string, CEventHandler *> map_handlers_;
    };
}

#endif