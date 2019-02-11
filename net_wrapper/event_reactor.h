#ifndef __LIBCOMMON_NET_WRAPPER_EVENT_REACTOR_H__
#define __LIBCOMMON_NET_WRAPPER_EVENT_REACTOR_H__

#include "uv.h"
#include <sstream>
#include "common.h"
#include "logger.h"
#include "exception.h"

namespace NetWrapper {
    class CReactorHandler;
    class CEventReactor {
    public:
        CEventReactor();
        ~CEventReactor();
        bool Register(CReactorHandler * handler);
        bool UnRegister(CReactorHandler * handler);
        void Dispatch();
        void LoopBreak();

        inline void Dispatch() {
            uv_run(&loop_, UV_RUN_NOWAIT);
        }

        inline void LoopBreak() {
            uv_stop(&loop_);
        }

        inline uv_loop_t * GetLibuvLoop() {
            return &loop_;
        }

    private:
        uv_loop_t loop_;
        uint32_t handler_count_;
    };
}

#endif