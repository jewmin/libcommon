#include "net_wrapper/event_reactor.h"
#include "net_wrapper/reactor_handler.h"

NetWrapper::CEventReactor::CEventReactor()
    : handler_count_(0) {
    OutputMsg(Logger::Info, "ÓÃĞÂµÄÍøÂçµ×²ã<libuv>");
    int err = uv_loop_init(&loop_);
    if (0 != err) {
        std::stringstream ss;
        ss << "uv_loop_init error[" << uv_strerror(err) << "]";
        throw BaseException(__func__, ss.str());
    }
}

NetWrapper::CEventReactor::~CEventReactor() {
    uv_loop_close(&loop_);
}

bool NetWrapper::CEventReactor::Register(CReactorHandler * handler) {
    bool ret = handler->RegisterToReactor();
    if (ret) {
        ++handler_count_;
    }
    return ret;
}

bool NetWrapper::CEventReactor::UnRegister(CReactorHandler * handler) {
    bool ret = handler->UnRegisterFromReactor();
    if (ret) {
        --handler_count_;
    }
    return ret;
}