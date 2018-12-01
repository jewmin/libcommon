#ifndef __LIBCOMMON_EVENT_LOOP_H__
#define __LIBCOMMON_EVENT_LOOP_H__

#include <vector>
#include <functional>

#include "uv.h"
#include "mutex.h"
#include "timer.h"
#include "common.h"
#include "logger.h"
#include "non_copy_able.hpp"

class EventLoop : public NonCopyAble {
public:
    using Callback = std::function<void()>;

    EventLoop(Logger * logger = nullptr);
    ~EventLoop();

    inline uv_loop_t * uv_loop() {
        return &loop_;
    }

    inline bool IsInLoopThread() {
        uv_thread_t current_thread = uv_thread_self();
        return !!uv_thread_equal(&thread_id_, &current_thread);
    }

    inline Logger * log() {
        return logger_;
    }

    // 在循环中执行回调
    void RunInLoop(const Callback & cb);
    // 加入回调队列
    void QueueInLoop(const Callback & cb);

    // 循环
    void Loop();
    // 退出循环
    void Quit();

    // 运行定时器
    uint32_t RunAt(uint64_t timeout_ms, uint64_t interval_ms, const Timer::TimerCallback & cb);
    // N毫秒后运行定时器
    uint32_t RunAfter(uint64_t delay_ms, const Timer::TimerCallback & cb);
    // 每隔N毫秒运行定时器
    uint32_t RunEvery(uint64_t interval_ms, const Timer::TimerCallback & cb);
    // 取消定时器
    void Cancel(uint32_t timer_id);

protected:
    // 执行回调
    void DoPendingCallbacks();

private:
    static void ThreadReqCb(uv_async_t * handle);
    static void WalkCb(uv_handle_t * handle, void * arg);

private:
    uv_loop_t loop_;
    uv_async_t thread_req_;
    uv_thread_t thread_id_;
    Mutex callbacks_lock_;
    std::vector<Callback> pending_callbacks_;
    TimerQueue * timer_queue_;
    Logger * logger_;
};

#endif