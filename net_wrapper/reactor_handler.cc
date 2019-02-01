#include "net_wrapper/reactor_handler.h"

NetWrapper::CReactorHandler::CReactorHandler(NetWrapper::CEventReactor * event_reactor)
    : event_reactor_(event_reactor) {

}

NetWrapper::CReactorHandler::~CReactorHandler() {
    event_reactor_ = nullptr;
}