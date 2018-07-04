#ifndef __LIB_COMMON_THREAD_H__
#define __LIB_COMMON_THREAD_H__

#include "3rd/libuv/include/uv.h"

class BaseThread
{
public:
    BaseThread();
    virtual ~BaseThread();
    int Start();
    void Stop();
    virtual void Run(void * arg) = 0;

    inline void Terminate() { _terminated = true; }
    inline bool IsTerminated() { return _terminated; }

protected:
    uv_thread_t _tid;
    bool _terminated;
};

#endif