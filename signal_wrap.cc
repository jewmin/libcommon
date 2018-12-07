#include "signal_wrap.h"
#include "event_loop.h"

SignalWrap::SignalWrap(EventLoop & loop, const int signum, const SignalCallback & cb)
    : loop_(loop), callback_(cb), signum_(signum) {
    signal_.data = nullptr;
}

void SignalWrap::Start() {
    assert(nullptr == signal_.data);

    signal_.data = this;
    int err = uv_signal_init(loop_.uv_loop(), uv_signal());
    assert(0 == err);

    err = uv_signal_start(uv_signal(), OnSignal, signum_);
    assert(0 == err);
}

void SignalWrap::Stop() {
    uv_signal_stop(uv_signal());
    uv_close(uv_handle(), OnsignalClosed);
}

SignalWrap * SignalWrap::Create(EventLoop & loop, const int signum, const SignalCallback & cb) {
    return new SignalWrap(loop, signum, cb);
}

void SignalWrap::OnSignal(uv_signal_t * handle, int signum) {
    SignalWrap * wrap = static_cast<SignalWrap *>(handle->data);
    if (wrap) {
        wrap->callback_(signum);
    }
}

void SignalWrap::OnsignalClosed(uv_handle_t * handle) {
    SignalWrap * wrap = static_cast<SignalWrap *>(handle->data);
    if (wrap) {
        delete wrap;
    }
}