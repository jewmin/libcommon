#include "event_loop.h"

EventLoop::EventLoop(Logger * logger)
    : timer_queue_(new TimerQueue(*this)), logger_(logger) {
    loop_.data = this;
    int err = uv_loop_init(&loop_);
    assert(0 == err);

    thread_req_.data = nullptr;
    err = uv_async_init(&loop_, &thread_req_, ThreadReqCb);
    assert(0 == err);

    thread_id_ = uv_thread_self();
}

EventLoop::~EventLoop() {
    delete timer_queue_;

    uv_walk(&loop_, WalkCb, nullptr);

    while (uv_run(&loop_, UV_RUN_NOWAIT)) {
        uv_run(&loop_, UV_RUN_ONCE);
    }

    uv_loop_close(&loop_);
}

void EventLoop::RunInLoop(const EventLoop::Callback & cb) {
    if (IsInLoopThread()) {
        cb();
    } else {
        QueueInLoop(cb);
    }
}

void EventLoop::QueueInLoop(const EventLoop::Callback & cb) {
    callbacks_lock_.Lock();
    pending_callbacks_.push_back(cb);
    callbacks_lock_.Unlock();

    if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(&thread_req_))) {
        uv_async_send(&thread_req_);
    }
}

void EventLoop::Loop() {
    uv_run(&loop_, UV_RUN_DEFAULT);
}

void EventLoop::Quit() {
    uv_stop(&loop_);
    if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(&thread_req_))) {
        uv_async_send(&thread_req_);
    }
}

uint32_t EventLoop::RunAt(uint64_t timeout_ms, uint64_t interval_ms, const Timer::TimerCallback & cb) {
    return timer_queue_->RegisterTimer(timeout_ms, interval_ms, cb);
}

uint32_t EventLoop::RunAfter(uint64_t delay_ms, const Timer::TimerCallback & cb) {
    return RunAt(delay_ms, 0, cb);
}

uint32_t EventLoop::RunEvery(uint64_t interval_ms, const Timer::TimerCallback & cb) {
    return RunAt(interval_ms, interval_ms, cb);
}

void EventLoop::Cancel(uint32_t timer_id) {
    timer_queue_->UnregisterTimer(timer_id);
}

void EventLoop::DoPendingCallbacks() {
    std::vector<Callback> callbacks;
    callbacks_lock_.Lock();
    callbacks.swap(pending_callbacks_);
    callbacks_lock_.Unlock();

    for (auto & cb : callbacks) {
        cb();
    }
}

void EventLoop::ThreadReqCb(uv_async_t * handle) {
    EventLoop * event_loop = static_cast<EventLoop *>(handle->loop->data);
    if (event_loop) {
        event_loop->DoPendingCallbacks();
    }
}

void EventLoop::WalkCb(uv_handle_t * handle, void * arg) {
    if (!uv_is_closing(handle)) {
        uv_close(handle, nullptr);
    }
}