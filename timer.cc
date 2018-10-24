#include "timer.h"
#include "event_loop.h"

std::atomic_uint32_t Timer::s_num_created_ = 0;

Timer::Timer(EventLoop & loop, TimerQueue & timers, uint64_t timeout_ms, uint64_t interval_ms, const TimerCallback & cb)
    : loop_(loop), timers_(timers), callback_(cb), interval_(interval_ms), sequence_(++s_num_created_) {
    uv_update_time(loop_.uv_loop());
    expiration_ = uv_now(loop_.uv_loop()) + timeout_ms;
    timer_.data = nullptr;
}

void Timer::Start() {
    assert(nullptr == timer_.data);

    timer_.data = this;
    int err = uv_timer_init(loop_.uv_loop(), uv_timer());
    assert(0 == err);

    // 从创建到开启这个过程，如果Loop()阻塞时间过长，可能会过期
    uv_update_time(loop_.uv_loop());
    uint64_t timeout = 0;
    uint64_t clamped_timeout = uv_now(loop_.uv_loop());
    if (clamped_timeout < expiration_) {
        timeout = expiration_ - clamped_timeout;
    }

    err = uv_timer_start(uv_timer(), OnTimer, timeout, interval_);
    assert(0 == err);
}

void Timer::Stop() {
    uv_timer_stop(uv_timer());
    uv_close(uv_handle(), OnTimerClosed);
}

Timer * Timer::Create(EventLoop & loop, TimerQueue & timers, uint64_t timeout_ms, uint64_t interval_ms, const TimerCallback & cb) {
    return new Timer(loop, timers, timeout_ms, interval_ms, cb);
}

void Timer::OnTimer(uv_timer_t * handle) {
    Timer * timer = static_cast<Timer *>(handle->data);
    if (timer) {
        timer->callback_();
        // 只运行一次，则反注册定时器
        if (0 == timer->interval_) {
            timer->timers_.UnregisterTimer(timer->sequence());
        }
    }
}

void Timer::OnTimerClosed(uv_handle_t * handle) {
    Timer * timer = static_cast<Timer *>(handle->data);
    if (timer) {
        delete timer;
    }
}

TimerQueue::TimerQueue(EventLoop & loop) 
    : loop_(loop) {

}

TimerQueue::~TimerQueue() {
    for (auto & it : timers_) {
        it.second->Stop();
    }
}

uint32_t TimerQueue::RegisterTimer(uint64_t timeout_ms, uint64_t interval_ms, const Timer::TimerCallback & cb) {
    Timer * timer = Timer::Create(loop_, *this, timeout_ms, interval_ms, cb);
    loop_.RunInLoop(std::bind(&TimerQueue::RegisterTimerInLoop, this, timer));
    return timer->sequence();
}

void TimerQueue::UnregisterTimer(uint32_t timer_id) {
    loop_.RunInLoop(std::bind(&TimerQueue::UnregisterTimerInLoop, this, timer_id));
}

void TimerQueue::RegisterTimerInLoop(Timer * timer) {
    timer->Start();
    TimerResult result = timers_.insert(TimerEntry(timer->sequence(), timer));
    assert(result.second);
}

void TimerQueue::UnregisterTimerInLoop(uint32_t timer_id) {
    TimerListIter it = timers_.find(timer_id);
    if (it != timers_.end()) {
        it->second->Stop();
        timers_.erase(it);
    }
}