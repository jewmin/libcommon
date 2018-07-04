#ifndef __LIB_COMMON_THREAD_H__
#define __LIB_COMMON_THREAD_H__

#include "uv.h"

class BaseThread
{
public:
    BaseThread();
    virtual ~BaseThread();
    int Start();
    void Stop();
    virtual void Run() = 0;

    inline void Terminate() { _terminated = true; }
    inline bool IsTerminated() { return _terminated; }

private:
    static void Callback(void * arg);

protected:
    uv_thread_t _tid;
    bool _terminated;
};

#endif