#ifndef __LIBCOMMON_EVENT_LOOP_THREAD_H__
#define __LIBCOMMON_EVENT_LOOP_THREAD_H__

#include "uv.h"
#include "thread.h"
#include "logger.h"
#include "event_loop.h"

class EventLoopThread : protected BaseThread {
public:
    EventLoopThread(Logger * logger = nullptr);
    ~EventLoopThread();
    EventLoop * StartLoop();

protected:
    void Run() override;

private:
    Logger * logger_;
    EventLoop * loop_;
    uv_sem_t thread_start_sem_;
    bool start_;
};

#endif