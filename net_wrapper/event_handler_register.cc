#include "net_wrapper/event_handler_register.h"

void NetWrapper::CEventHandlerRegister::RegisterHandler(CEventHandler * handler) {
    map_handlers_.insert(std::make_pair(handler->GetName(), handler));
}

void NetWrapper::CEventHandlerRegister::UnRegisterAll() {
    for (std::map<std::string, CEventHandler *>::iterator it = map_handlers_.begin(); it != map_handlers_.end(); ++it) {
        delete it->second;
    }
    map_handlers_.clear();
}