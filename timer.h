#ifndef __LIBCOMMON_TIMER_H__
#define __LIBCOMMON_TIMER_H__

#include <map>
#include <atomic>
#include <functional>

#include "uv.h"
#include "non_copy_able.hpp"

class EventLoop;
class TimerQueue;
class Timer : public NonCopyAble {
public:
    using TimerCallback = std::function<void()>;

    inline uv_timer_t * uv_timer() {
        return &timer_;
    }

    inline uv_handle_t * uv_handle() {
        return reinterpret_cast<uv_handle_t *>(&timer_);
    }

    inline uint32_t sequence() const {
        return sequence_;
    }

    void Start();
    void Stop();
    static Timer * Create(EventLoop & loop, TimerQueue & timers, uint64_t timeout_ms, uint64_t interval_ms, const TimerCallback & cb);

protected:
    Timer(EventLoop & loop, TimerQueue & timers, uint64_t timeout_ms, uint64_t interval_ms, const TimerCallback & cb);

private:
    static void OnTimer(uv_timer_t * handle);
    static void OnTimerClosed(uv_handle_t * handle);

private:
    EventLoop & loop_;
    TimerQueue & timers_;
    uv_timer_t timer_;
    const TimerCallback callback_;
    uint64_t expiration_;
    const uint64_t interval_;
    const uint32_t sequence_;
    static std::atomic<uint32_t> s_num_created_;
};

class TimerQueue : public NonCopyAble {
    using TimerEntry = std::pair<uint32_t, Timer *>;
    using TimerList = std::map<uint32_t, Timer *>;
    using TimerListIter = std::map<uint32_t, Timer *>::iterator;
    using TimerResult = std::pair<TimerListIter, bool>;

public:
    explicit TimerQueue(EventLoop & loop);
    ~TimerQueue();

    uint32_t RegisterTimer(uint64_t timeout_ms, uint64_t interval_ms, const Timer::TimerCallback & cb);
    void UnregisterTimer(uint32_t timer_id);

protected:
    void RegisterTimerInLoop(Timer * timer);
    void UnregisterTimerInLoop(uint32_t timer_id);

private:
    EventLoop & loop_;
    TimerList timers_;
};

#endif