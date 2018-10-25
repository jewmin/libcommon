#include "event_loop_thread.h"

EventLoopThread::EventLoopThread(Logger * logger)
    : logger_(logger), loop_(nullptr), start_(false) {
    uv_sem_init(&thread_start_sem_, 0);
}

EventLoopThread::~EventLoopThread() {
    uv_sem_destroy(&thread_start_sem_);
    if (loop_) {
        loop_->Quit();
        Stop();
    }
}

EventLoop * EventLoopThread::StartLoop() {
    assert(!start_);

    int err = Start();
    if (0 == err) {
        start_ = true;
        uv_sem_wait(&thread_start_sem_);
    }

    return loop_;
}

void EventLoopThread::Run() {
    EventLoop loop(logger_);

    loop_ = &loop;
    uv_sem_post(&thread_start_sem_);
    loop.Loop();
    loop_ = nullptr;
}