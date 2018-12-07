#ifndef __LIBCOMMON_SIGNAL_WRAP_H__
#define __LIBCOMMON_SIGNAL_WRAP_H__

#include <functional>

#include "uv.h"
#include "non_copy_able.hpp"

class EventLoop;
class SignalWrap : public NonCopyAble {
public:
    using SignalCallback = std::function<void(int)>;

    inline uv_signal_t * uv_signal() {
        return &signal_;
    }

    inline uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&signal_);
    }

    void Start();
    void Stop();
    static SignalWrap * Create(EventLoop & loop, const int signum, const SignalCallback & cb);

protected:
    SignalWrap(EventLoop & loop, const int signum, const SignalCallback & cb);

private:
    static void OnSignal(uv_signal_t * handle, int signum);
    static void OnsignalClosed(uv_handle_t * handle);

private:
    EventLoop & loop_;
    uv_signal_t signal_;
    const SignalCallback callback_;
    const int signum_;
};

#endif